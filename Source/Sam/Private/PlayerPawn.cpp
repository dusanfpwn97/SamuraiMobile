// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Misc/HelperLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../SamGameMode.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/EnemySpawnerComponent.h"
#include "Misc/BaseWeapon.h"
#include "Animation/AnimMontage.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Misc/SlashIndicator.h"


APlayerPawn::APlayerPawn()
{
	//CapsuleSiz
	GetCapsuleComponent()->SetCapsuleSize(42, 96, false);
	RootComponent = GetCapsuleComponent();
	
	TargetArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	TargetArm->SetupAttachment(RootComponent);
	TargetArm->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	TargetArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(TargetArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void APlayerPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	
	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed,  this, &APlayerPawn::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &APlayerPawn::TouchStopped);
}

void APlayerPawn::SetStartingValues()
{
	StartingSpringArmLength = TargetArm->TargetArmLength;
	StartingSpringOffset = TargetArm->SocketOffset;
	StartingSpringArmRot = TargetArm->GetComponentRotation();

	StartRunning();
	
}

void APlayerPawn::MoveTowardsTarget()
{
	UWorld* World = GetWorld();


	if (!World) return;

	if (!CurrentTarget)
	{
		SetNextTarget();
		if (!CurrentTarget) return;
	}
	else
	{
		AEnemyBase* sss = (AEnemyBase*)CurrentTarget;

		if (sss)
		{
			if (!sss->IsAlive)
			{
				SetNextTarget();
				if (!CurrentTarget) return;
			}
		}
		else
		{
			SetNextTarget();
			if (!CurrentTarget) return;
		}
	}

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	TargetLocation += GetActorRightVector() * -26.f;
	//TargetLocation += GetActorForwardVector() * -166.f;

	float Distance = FVector::Dist(TargetLocation, GetActorLocation());

	//if (Distance < 120) return;

	FVector Direction = TargetLocation - GetActorLocation();
	Direction.Normalize();

	LastDirection = FMath::VInterpTo(LastDirection, Direction, World->DeltaTimeSeconds, 7.f);

	if (Distance < 150) return;



	FVector DeltaLoc = LastDirection * CurrentSpeed * World->DeltaTimeSeconds;
	AddActorWorldOffset(DeltaLoc, true);

}

void APlayerPawn::SetNextTarget()
{
	if (IsAttacking()) return;
	UWorld* World = GetWorld();
	if (!World) return;
	ASamGameMode* SamGM = (ASamGameMode*)UGameplayStatics::GetGameMode(World);
	if (!SamGM) return;
	UEnemySpawnerComponent* SpawnerComp = (UEnemySpawnerComponent*)SamGM->GetComponentByClass(UEnemySpawnerComponent::StaticClass());
	if (!SpawnerComp) return;

	TArray<AActor*> FoundActors;

	for (AEnemyBase* Enemy : SpawnerComp->SpawnedEnemies)
	{
		if (Enemy)
		{
			if (Enemy->IsAlive)
			{
				FoundActors.Add(Enemy);
			}
		}
	}

	if (FoundActors.Num() == 0)
	{
		CurrentTarget = nullptr;
		return;
	}

	CurrentTarget = UHelperLibrary::GetClosestActor(FoundActors, GetActorLocation());
}

void APlayerPawn::UpdateRotation()
{
	//if (ActionState == EActionState::ATTACKING || ActionState == EActionState::PREPARING_FOR_ATTACK) return;

	SetActorRotation(LastDirection.Rotation());

}

void APlayerPawn::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{

	StartStartingDash();
}

void APlayerPawn::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartAttacking(true);
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	MoveTowardsTarget();
	UpdateRotation();

	DoLoops();

	CheckPrepareToAttack();
	CameraAttackMovementLoop();

}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	
	SetStartingValues();
}

void APlayerPawn::EquipNewWeapon(TSoftClassPtr<ABaseWeapon> WepClass)
{
	WepClass.LoadSynchronous();
	if (CurrentWeapon) CurrentWeapon->Destroy();

	UWorld* World = GetWorld();
	if (!WepClass.IsValid() || !World) return;

	UClass* WeaponClass = WepClass.Get();

	FTransform Transform;
	Transform.SetScale3D(FVector(1.f, 1.f, 1.f));
	Transform.SetLocation(GetActorLocation());

	ABaseWeapon* Wep = (ABaseWeapon*)World->SpawnActor<ABaseWeapon>(WeaponClass, Transform);
	
	if (!Wep)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Could not spawn weapon!")));
		return;
	}
	FAttachmentTransformRules Rules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;


	Wep->AttachToComponent(GetMesh(), Rules, FName("Katana_r"));
	CurrentWeapon = Wep;

	Wep->Owner = this;
}


void APlayerPawn::CheckPrepareToAttack()
{
	if (ActionState == EActionState::PREPARING_TO_ATTACK/* || !IsDashing()*/) return;

	if (!CurrentTarget) return;
	if (PrepareForAttackTarget == CurrentTarget) return;

	if (FVector::Distance(CurrentTarget->GetActorLocation(), GetActorLocation()) > AttackInfo.DistanceForSlowdown) return;

	StartPreparingToAttack();
}


void APlayerPawn::OnDamageNotifyStarted()
{
	if (/*ActionState != EActionState::ATTACKING || */!CurrentWeapon || !CurrentTarget) return;
	
	AEnemyBase* Enemy = (AEnemyBase*)CurrentTarget;
	if (!Enemy) return;
	USkeletalMeshComponent * SKMComp = Enemy->SkelMesh;
	if (!SKMComp) return;

	CurrentWeapon->StartCheckingCollision();

	//GetMesh()->SetPlayRate(0.01);
	CustomTimeDilation = 0.5;

}

void APlayerPawn::OnDamageNotifyEnded()
{
	if (CurrentWeapon) CurrentWeapon->StopCheckingCollision();

	CustomTimeDilation = 1;
}

bool APlayerPawn::IsAttacking()
{
	if (ActionState == EActionState::ATTACKING || ActionState == EActionState::ATTACKING_HIT)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void APlayerPawn::OnWeaponHitEnemy(AActor* Actor, FName Bone)
{

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Enemy Hit")));
	
	// TODO this will cause problems for multiple enemies
	CurrentWeapon->StopCheckingCollision();
	CurrentTarget = nullptr;

	StartAttackingHit();
	//CustomTimeDilation = 1;
}

void APlayerPawn::StartCameraAttackMovement(ECameraMovement CamMovement)
{
	CameraMovement = CamMovement;
	TargetArm->CameraLagSpeed = 30.f;
	TargetArm->CameraRotationLagSpeed = 30.f;
	if (CamMovement == ECameraMovement::FORWARD)
	{
		CameraControlAccumulatedTime = 0;
	}
}

void APlayerPawn::CameraAttackMovementLoop()
{
	if(CameraMovement == ECameraMovement::NOT_MOVING) return;

	UWorld* World = GetWorld(); if (!World) return;
	if (!AttackInfo.CameraZoomCurve || !AttackInfo.CameraRotationCurve || !AttackInfo.CameraLocationCurve) return;
	
	TArray<float> MaxTimes;
	float ZoomMinTime, ZoomMaxTime, RotMinTime, RotMaxTime, LocMinTime, LocMaxTime, MaxTime;
	float NewZoomVal;
	ZoomMinTime = ZoomMaxTime = RotMinTime = RotMinTime = RotMaxTime = LocMinTime = LocMaxTime = MaxTime = NewZoomVal = 0;
	
	FVector NewRotOffset	  = FVector::ZeroVector;
	FVector NewSocketOffset = FVector::ZeroVector;

	if (AttackInfo.CameraZoomCurve)
	{
		AttackInfo.CameraZoomCurve->GetTimeRange(ZoomMinTime, ZoomMaxTime);
		NewZoomVal = AttackInfo.CameraZoomCurve->GetFloatValue(CameraControlAccumulatedTime);
	}
	if (AttackInfo.CameraRotationCurve)
	{
		AttackInfo.CameraRotationCurve->GetTimeRange(RotMinTime, RotMaxTime);
		NewRotOffset = AttackInfo.CameraRotationCurve->GetVectorValue(CameraControlAccumulatedTime);
	}
	if (AttackInfo.CameraLocationCurve)
	{
		AttackInfo.CameraLocationCurve->GetTimeRange(LocMinTime, LocMaxTime);
		NewSocketOffset = AttackInfo.CameraLocationCurve->GetVectorValue(CameraControlAccumulatedTime);
	}

	MaxTime = FMath::Max(ZoomMaxTime, RotMaxTime);
	MaxTime = FMath::Max(MaxTime, LocMaxTime);

	if (CameraMovement == ECameraMovement::FORWARD)
	{
		if (CameraControlAccumulatedTime > MaxTime)
		{
			StopCameraAttackMovement();
			return;
		}
		else
		{
			CameraControlAccumulatedTime += World->DeltaRealTimeSeconds;
		}
	}
	else if (CameraMovement == ECameraMovement::BACKWARD)
	{
		if (CameraControlAccumulatedTime < 0)
		{
			StopCameraAttackMovement();
			return;
		}
		else
		{
			CameraControlAccumulatedTime -= World->DeltaRealTimeSeconds;
		}
	}
	else
	{
		StopCameraAttackMovement();
		return;
	}

	TargetArm->TargetArmLength = StartingSpringArmLength + NewZoomVal;
	TargetArm->SocketOffset = StartingSpringOffset + NewSocketOffset;

	FRotator Rot;
	Rot.Roll = NewRotOffset.X;
	Rot.Pitch = NewRotOffset.Y;
	Rot.Yaw = NewRotOffset.Z;

	const float Alpha = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxTime), FVector2D(0, 1), CameraControlAccumulatedTime);

	//TargetArm->SetRelativeRotation(FMath::Lerp(StartingSpringArmRot, Rot + StartingSpringArmRot, Alpha));
	TargetArm->SetRelativeRotation(Rot + StartingSpringArmRot);

	

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%s"), *FString::SanitizeFloat(World->DeltaRealTimeSeconds)));
}

void APlayerPawn::StopCameraAttackMovement()
{
	CameraMovement = ECameraMovement::NOT_MOVING;
	TargetArm->CameraLagSpeed = 6.f;
	TargetArm->CameraRotationLagSpeed = 6.f;
}

void APlayerPawn::DoLoops()
{
	if (ActionState == EActionState::STARTING_DASH)
	{
		StartingDashLoop();
		return;
	}
	else if (ActionState == EActionState::DASHING)
	{
		DashingLoop();
		return;
	}
	else if (ActionState == EActionState::PREPARING_TO_ATTACK)
	{
		PreparingToAttackLoop();
		return;
	}
	else if (ActionState == EActionState::ATTACKING)
	{
		AttackingLoop();
		return;
	}
	else if (ActionState == EActionState::ATTACKING_HIT)
	{
		AttackingHitLoop();
		return;
	}
	else if (ActionState == EActionState::RUNNING)
	{
		RunningLoop();
		return;
	}
}

void APlayerPawn::StartStartingDash()
{

	if (!AttackInfo.StartDashSpeedCurve || !AttackInfo.StartDashMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Starting Dash cannot be started. Something is not set. PlayerPawn->StartDashing()")));
		return;
	}

	ActionState = EActionState::STARTING_DASH;
	DashAccumulatedTime = 0.f;

	float MontageDuration = PlayAnimMontage(AttackInfo.StartDashMontage);

	//float MinTime, MaxTime;
	//AttackInfo.StartDashSpeedCurve->GetTimeRange(MinTime, MaxTime);

}

void APlayerPawn::StartingDashLoop()
{
	UWorld* World = GetWorld(); if (!World) return;
	if (!AttackInfo.StartDashSpeedCurve) return;

	float MinTime, MaxTime;
	AttackInfo.StartDashSpeedCurve->GetTimeRange(MinTime, MaxTime);


	CurrentSpeed = AttackInfo.StartDashSpeedCurve->GetFloatValue(DashAccumulatedTime) * RunSpeed;
	DashAccumulatedTime += World->DeltaRealTimeSeconds;

	if (DashAccumulatedTime > MaxTime)
	{
		StartDashing();
	}
}

void APlayerPawn::StartDashing()
{

	ActionState = EActionState::DASHING;
	DashAccumulatedTime = 0.f;

}

void APlayerPawn::DashingLoop()
{
	UWorld* World = GetWorld(); if (!World) return;
	if (ActionState != EActionState::DASHING) return;

	if (!DashSpeedCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Trying to dash but StartDashSpeedCurve is null")));
		return;
	}

	CurrentSpeed = DashSpeedCurve->GetFloatValue(DashAccumulatedTime) * RunSpeed;
	DashAccumulatedTime += World->DeltaRealTimeSeconds;
}

void APlayerPawn::StartPreparingToAttack()
{
	UWorld* World = GetWorld(); if (!World) return;
	ActionState = EActionState::PREPARING_TO_ATTACK;
	PrepareForAttackTarget = CurrentTarget;

	CustomTimeDilation = 0.1;

	FTransform Transform;
	Transform.SetScale3D(FVector(1.f, 1.f, 1.f));
	Transform.SetLocation(PrepareForAttackTarget->GetActorLocation() + FVector(0, 0, 30.f));

	ASlashIndicator* Indicator = (ASlashIndicator*)World->SpawnActor<ASlashIndicator>(SlashIndicatorClass, Transform);


	StartCameraAttackMovement(ECameraMovement::FORWARD);
	//FAttachmentTransformRules Rules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;

	//Wep->AttachToActor(CurrentPrepareForAttackTarget, Rules);
}

void APlayerPawn::PreparingToAttackLoop()
{
	if (ActionState != EActionState::PREPARING_TO_ATTACK) return;
	CurrentSpeed = PrepareToAttackSpeed;
}

void APlayerPawn::StartAttacking(bool Reset)
{
	UWorld* World = GetWorld();
	if (!World || !AttackInfo.AttackMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Attack cannot be started. Something is null. PlayerPawn->StartAttacking()")));
		return;
	}

	ActionState = EActionState::ATTACKING;
	CustomTimeDilation = 1.f;


	if (Reset)
	{
		AttackAccumulatedTime = 0.f;
		float MontageDuration = PlayAnimMontage(AttackInfo.AttackMontage);
	}
	else
	{

	}




	//float MinTime, MaxTime;
	//HitSlowMoCurve->GetTimeRange(MinTime, MaxTime);

	//MontageDuration *= 1+(1-AttackInfo.AttackMontage->RateScale) + MaxTime;

}

void APlayerPawn::AttackingLoop()
{
	if (ActionState != EActionState::ATTACKING) return;
	CurrentSpeed = 0.f;


	AdvanceAttackTime();

}
void APlayerPawn::AdvanceAttackTime()
{
	if (!IsAttacking() || AttackAccumulatedTime < 0) return;
	UWorld* World = GetWorld(); if (!World) return;
	float MontageDuration = AttackInfo.AttackMontage->GetPlayLength();

	//const int32 CurrentFrame = AttackInfo.AttackMontage->GetFrameAtTime(StartRunningAccumulated);
	//const float CurrentTime = AttackInfo.AttackMontage->GetTimeAtFrame(CurrentFrame);

	if (AttackAccumulatedTime >= MontageDuration)
	{
		StopAttack();
	}
	AttackAccumulatedTime += World->DeltaTimeSeconds * CustomTimeDilation;

}

void APlayerPawn::StopAttack()
{
	OnAttackEnded();
	StartRunning();
	StartCameraAttackMovement(ECameraMovement::BACKWARD);
	AttackAccumulatedTime = -1;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Stop attack")));
}



void APlayerPawn::StartAttackingHit()
{

	CurrentHitSlowMoAccumulatedTime = 0.f;
	ActionState = EActionState::ATTACKING_HIT;
}

void APlayerPawn::AttackingHitLoop()
{

	UWorld* World = GetWorld(); if (!World) return;
	if (ActionState != EActionState::ATTACKING_HIT) return;
	//if (CurrentHitSlowMoTimeAccumulated < 0.f) return;

	float MinTime, MaxTime;
	HitSlowMoCurve->GetTimeRange(MinTime, MaxTime);
	CurrentHitSlowMoAccumulatedTime += World->DeltaRealTimeSeconds;

	float TempSlowMoVal = HitSlowMoCurve->GetFloatValue(CurrentHitSlowMoAccumulatedTime);
	CustomTimeDilation = TempSlowMoVal;

	CurrentSpeed = 0;

	AdvanceAttackTime();


	if (CurrentHitSlowMoAccumulatedTime > MaxTime)
	{
		StartAttacking(false);
	}



}

void APlayerPawn::StartRunning()
{
	ActionState = EActionState::RUNNING;
	AttackAccumulatedTime = 0;
}

void APlayerPawn::RunningLoop()
{
	if (ActionState != EActionState::RUNNING) return;
	CurrentSpeed = RunSpeed;
}


bool APlayerPawn::IsDashing()
{
	if (ActionState == EActionState::DASHING || ActionState == EActionState::STARTING_DASH)
	{
		return true;
	}

	return false;
}