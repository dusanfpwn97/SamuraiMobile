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
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCam.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerPawn::APlayerPawn()
{
	//CapsuleSiz
	GetCapsuleComponent()->SetCapsuleSize(42, 96, false);
	RootComponent = GetCapsuleComponent();
	
	
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
	//AddActorWorldOffset(DeltaLoc, true);

	AddMovementInput(LastDirection, 1);
	
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
	
	SetActorRotation(LastDirection.Rotation().Quaternion());
	//GetCharacterMovement()->Rotat
	
}

void APlayerPawn::SpawnCamera()
{
	UWorld* World = GetWorld();  if(!World) return;
	//if (!CameraSoftClass.IsValid()) return;
	FTransform Transform;
	Transform.SetLocation(GetActorLocation());
	Transform.SetRotation(GetActorRotation().Quaternion());
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CameraSoftClass.LoadSynchronous();
	UClass* TempCamClass = CameraSoftClass.Get();
	if (!TempCamClass) return;
	Camera = (APlayerCam*)World->SpawnActor<APlayerCam>(TempCamClass, Transform, Params);
	if (!Camera)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Could not spawn Camera!!! Should not happen ever")));
		return;
	}
	Camera->SetPlayer(this);

	BP_OnCameraCreated();

}

void APlayerPawn::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartStartingDash();
}

void APlayerPawn::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartAttacking();
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveTowardsTarget();
	UpdateRotation();
	
	DoLoops();
	
	CheckPrepareToAttack();
	


}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	SpawnCamera();
	
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

const bool APlayerPawn::IsAttacking()
{
	if (ActionState == EActionState::ATTACKING ||
		ActionState == EActionState::ATTACKING_HIT ||
		ActionState == EActionState::ENDING_ATTACK)
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
	else if (ActionState == EActionState::ENDING_ATTACK)
	{
		EndingAttackLoop();
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
	GetCharacterMovement()->MaxWalkSpeed = 0;
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

	GetCharacterMovement()->MaxWalkSpeed = 2600;

	Camera->StartDash();
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

	CustomTimeDilation = 0.033;

	FTransform Transform;
	Transform.SetScale3D(FVector(1.f, 1.f, 1.f));
	Transform.SetLocation(PrepareForAttackTarget->GetActorLocation() + FVector(0, 0, 30.f));

	ASlashIndicator* Indicator = (ASlashIndicator*)World->SpawnActor<ASlashIndicator>(SlashIndicatorClass, Transform);
	
	Camera->StartPreparingToAttack(true);

	GetCharacterMovement()->MaxWalkSpeed = 0;


	//FAttachmentTransformRules Rules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;

	//Wep->AttachToActor(CurrentPrepareForAttackTarget, Rules);
}

void APlayerPawn::PreparingToAttackLoop()
{
	if (ActionState != EActionState::PREPARING_TO_ATTACK) return;
	CurrentSpeed = PrepareToAttackSpeed;
}

void APlayerPawn::StartAttacking()
{
	UWorld* World = GetWorld();
	if (!World || !AttackInfo.AttackMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Attack cannot be started. Something is null. PlayerPawn->StartAttacking()")));
		return;
	}

	ActionState = EActionState::ATTACKING;
	CustomTimeDilation = 1.f;

	AttackAccumulatedTime = 0.f;
	float MontageDuration = PlayAnimMontage(AttackInfo.AttackMontage);
	
	Camera->StartAttacking();

	GetCharacterMovement()->MaxWalkSpeed = 0;

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
		StopAttackLoop();
	}
	AttackAccumulatedTime += World->DeltaTimeSeconds * CustomTimeDilation;

}

void APlayerPawn::StopAttackLoop()
{
	OnAttackEnded();
	StartRunning();

	AttackAccumulatedTime = -1;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Stop attack")));


}



void APlayerPawn::StartAttackingHit()
{


	CurrentHitSlowMoAccumulatedTime = 0.f;
	ActionState = EActionState::ATTACKING_HIT;
	Camera->StartAttackingHit();

	GetCharacterMovement()->MaxWalkSpeed = 0;
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
		StartEndingAttack();
	}

}

void APlayerPawn::StartEndingAttack()
{
	ActionState = EActionState::ENDING_ATTACK;
	CustomTimeDilation = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = 0;
	Camera->StartEndingAttack();
}

void APlayerPawn::EndingAttackLoop()
{
	CurrentSpeed = 0.f;
	AdvanceAttackTime();
}

void APlayerPawn::StartRunning()
{
	ActionState = EActionState::RUNNING;
	AttackAccumulatedTime = 0;
	GetCharacterMovement()->MaxWalkSpeed = 600;
	Camera->StartRunning();
}

void APlayerPawn::RunningLoop()
{
	if (ActionState != EActionState::RUNNING) return;
	CurrentSpeed = RunSpeed;
}

const bool APlayerPawn::IsDashing()
{
	if (ActionState == EActionState::DASHING || ActionState == EActionState::STARTING_DASH)
	{
		return true;
	}

	return false;
}

