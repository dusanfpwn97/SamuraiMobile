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
	//StartPreparingToDash();
	//StartRunning();
	
}

void APlayerPawn::MoveTowardsDirection()
{

	//AddActorWorldOffset(DeltaLoc, true);

	AddMovementInput(LastDirection, 1);
	
}

void APlayerPawn::SetNextTarget()
{
	//if (IsAttacking()) return;
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
	// only react to first finger that touches the screen
	if (FingerIndex != ETouchIndex::Type::Touch1) return;

	if (!StartedAttackingGame)
	{
		SetNextTarget();
		StartDashing();
		StartedAttackingGame = true;
		return;
	}

	if (ActionState == EActionState::STARTING_ATTACK)
	{
		TryToSlash();
	}
}

void APlayerPawn::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartTouches.Remove(FingerIndex);
	CurrentTouches.Remove(FingerIndex);
	//StartAttacking();
}

void APlayerPawn::UpdateSwipeDirection()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (IsAttacking()) return;

	APlayerController* PC = World->GetFirstPlayerController();

	if (!CurrentTouches.Contains(ETouchIndex::Type::Touch1)) return;

	float LocX, LocY;
	bool bIsPressed;
	PC->GetInputTouchState(ETouchIndex::Type::Touch1, LocX, LocY, bIsPressed);

	CurrentTouches.Add(ETouchIndex::Type::Touch1, FVector(LocX, LocY, 0));

	SwipeDirection.Y = (CurrentTouches.FindRef(ETouchIndex::Type::Touch1).X - StartTouches.FindRef(ETouchIndex::Type::Touch1).X);
	SwipeDirection.X = (CurrentTouches.FindRef(ETouchIndex::Type::Touch1).Y - StartTouches.FindRef(ETouchIndex::Type::Touch1).Y);

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("%s"), *FString::SanitizeFloat(SwipeDirection.Length())));

	if (SwipeDirection.Length() > 70.f && !IsAttacking())
	{
		SetNextTarget();
		StartDashing();
		SwipeDirection = FVector();
		CurrentTouches.Remove(ETouchIndex::Type::Touch1);
		StartTouches.Remove(ETouchIndex::Type::Touch1);
	}

}

bool APlayerPawn::GetTargetUnderFinger(ETouchIndex::Type FingerIndex)
{
	UWorld* World = GetWorld(); if (!World) return false;

	FHitResult Hit;

	// TODO maybe orient towards slash?
	World->GetFirstPlayerController()->GetHitResultUnderFinger(FingerIndex, ECollisionChannel::ECC_GameTraceChannel3, false, Hit);

		
	
	if (!Hit.bBlockingHit) return false;
	
	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return false;


	CurrentTarget = HitActor;
	return false;
	
}



void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//MoveTowardsTarget();
	//UpdateSwipeDirection();

	
	DoLoops();
	
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

void APlayerPawn::TryToSlash()
{
	if (SlashIndicator)
	{
		SlashIndicator->Destroy();
	}

	CustomTimeDilation = 0.75;
}

void APlayerPawn::OnDamageNotifyStarted()
{
	UWorld* World = GetWorld(); if (!World) return;
	if (/*ActionState != EActionState::ATTACKING || */!CurrentWeapon || !CurrentTarget) return;
	
	AEnemyBase* Enemy = (AEnemyBase*)CurrentTarget;
	if (!Enemy) return;
	USkeletalMeshComponent * SKMComp = Enemy->SkelMesh;
	if (!SKMComp) return;

	CurrentWeapon->StartCheckingCollision();

	//GetMesh()->SetPlayRate(0.01);
	CustomTimeDilation = 0.02;


	FTransform Transform;
	Transform.SetScale3D(FVector(1.f, 1.f, 1.f));
	Transform.SetLocation(CurrentTarget->GetActorLocation() + FVector(0, 0, 30.f));

	SlashIndicator = (ASlashIndicator*)World->SpawnActor<ASlashIndicator>(SlashIndicatorClass, Transform);


}

void APlayerPawn::OnDamageNotifyEnded()
{
	if (CurrentWeapon) CurrentWeapon->StopCheckingCollision();

	//StopAttackLoop();
	CustomTimeDilation = 1;
}


const bool APlayerPawn::IsAttacking()
{
	if (ActionState == EActionState::STARTING_ATTACK ||
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

	if (ActionState == EActionState::DASHING)
	{
		DashingLoop();
		return;
	}
	else if (ActionState == EActionState::PREPARING_TO_ATTACK)
	{
		PreparingToAttackLoop();
		return;
	}
	else if (ActionState == EActionState::STARTING_ATTACK)
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
}

void APlayerPawn::StartDashing()
{
	LastAttackedTarget = nullptr;
	SetNextTarget();
	ActionState = EActionState::DASHING;
	DashAccumulatedTime = 0.f;

	//GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	//MoveTowardsTarget();

	Camera->StartDash();
}

void APlayerPawn::DashingLoop()
{
	UWorld* World = GetWorld(); if (!World) return;

	if (!DashSpeedCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Trying to dash but StartDashSpeedCurve is null")));
		return;
	}

	UpdateDirection();

	GetCharacterMovement()->MaxWalkSpeed = DashSpeedCurve->GetFloatValue(DashAccumulatedTime) * RunSpeed;
	DashAccumulatedTime += World->DeltaRealTimeSeconds;

	MoveTowardsDirection();
	SetActorRotation(LastDirection.Rotation().Quaternion());



	//
	if (IsAttacking()) return;
	if (!CurrentTarget) return;
	if (LastAttackedTarget == CurrentTarget) return;
	if (FVector::Distance(CurrentTarget->GetActorLocation(), GetActorLocation()) > AttackInfo.DistanceForSlowdown) return;
	StartAttacking();

}

void APlayerPawn::StartPreparingToAttack()
{
	//UWorld* World = GetWorld(); if (!World) return;
	//ActionState = EActionState::PREPARING_TO_ATTACK;
	//PrepareForAttackTarget = CurrentTarget;
	//
	//CustomTimeDilation = 1;
	//
	//FTransform Transform;
	//Transform.SetScale3D(FVector(1.f, 1.f, 1.f));
	//Transform.SetLocation(PrepareForAttackTarget->GetActorLocation() + FVector(0, 0, 30.f));
	//
	//ASlashIndicator* Indicator = (ASlashIndicator*)World->SpawnActor<ASlashIndicator>(SlashIndicatorClass, Transform);
	//
	//Camera->StartPreparingToAttack(true);
	//
	//GetCharacterMovement()->MaxWalkSpeed = 0;
	//
	//
	//FAttachmentTransformRules Rules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	//
	//Wep->AttachToActor(CurrentPrepareForAttackTarget, Rules);
}

void APlayerPawn::PreparingToAttackLoop()
{
	if (ActionState != EActionState::PREPARING_TO_ATTACK) return;

	GetCharacterMovement()->MaxWalkSpeed = 0;
}	

void APlayerPawn::StartAttacking()
{
	UWorld* World = GetWorld();
	if (!World || !AttackInfo.AttackMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Attack cannot be started. AttackMontage is null. PlayerPawn->StartAttacking()")));
		return;
	}

	CustomTimeDilation = 1.f;

	LastAttackedTarget = CurrentTarget;

	GetCharacterMovement()->MaxWalkSpeed = 0;

	ActionState = EActionState::STARTING_ATTACK;

	AttackAccumulatedTime = 0.0001f;
	float MontageDuration = PlayAnimMontage(AttackInfo.AttackMontage);
	
	Camera->StartAttacking();

	GetCharacterMovement()->MaxWalkSpeed = 0;

	//float MinTime, MaxTime;
	//HitSlowMoCurve->GetTimeRange(MinTime, MaxTime);

	//MontageDuration *= 1+(1-AttackInfo.AttackMontage->RateScale) + MaxTime;

}

void APlayerPawn::AttackingLoop()
{

	AdvanceAttackTime();

}
void APlayerPawn::AdvanceAttackTime()
{
	if (!IsAttacking() || AttackAccumulatedTime < 0) return;
	UWorld* World = GetWorld(); if (!World) return;
	float MontageDuration = AttackInfo.AttackMontage->GetPlayLength();

	//const int32 CurrentFrame = AttackInfo.AttackMontage->GetFrameAtTime(StartRunningAccumulated);
	//const float CurrentTime = AttackInfo.AttackMontage->GetTimeAtFrame(CurrentFrame);
	//if (AttackInfo.StartAttackSlomoCurve)
	//{
	//	float TempSlowMoVal = AttackInfo.StartAttackSlomoCurve->GetFloatValue(AttackAccumulatedTime);
	//	CustomTimeDilation = TempSlowMoVal;
	//}


	if (AttackAccumulatedTime >= MontageDuration)
	{
		StopAttackLoop();
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%s "), *FString::SanitizeFloat(CustomTimeDilation)));

	AttackAccumulatedTime += World->DeltaTimeSeconds * CustomTimeDilation;

}

void APlayerPawn::StopAttackLoop()
{
	OnAttackEnded();
	//StartRunning();

	AttackAccumulatedTime = -1;
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Stop attack")));

	SetNextTarget();
	StartDashing();
	//StartPreparingToDash();

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
	//if (ActionState != EActionState::ATTACKING_HIT) return;
	//if (CurrentHitSlowMoTimeAccumulated < 0.f) return;

	float MinTime, MaxTime;
	HitSlowMoCurve->GetTimeRange(MinTime, MaxTime);
	CurrentHitSlowMoAccumulatedTime += World->DeltaRealTimeSeconds;

	float TempSlowMoVal = HitSlowMoCurve->GetFloatValue(CurrentHitSlowMoAccumulatedTime);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s "), *FString::SanitizeFloat(TempSlowMoVal)));
	CustomTimeDilation = TempSlowMoVal;

	//AdvanceAttackTime();

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
	AdvanceAttackTime();
}

void APlayerPawn::UpdateDirection()
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

	//if (Distance < AttackInfo.DistanceForSlowdown) return;



	//FVector DeltaLoc = LastDirection * CurrentSpeed * World->DeltaTimeSeconds;
}

const bool APlayerPawn::IsDashing()
{
	if (ActionState == EActionState::DASHING)
	{
		return true;
	}

	return false;
}

