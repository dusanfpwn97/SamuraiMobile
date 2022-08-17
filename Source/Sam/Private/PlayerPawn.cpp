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

	if (ActionState == EActionState::ATTACKING && CurrentHitStage == EHitStage::NONE)
	{
		Hit();
	}
}

void APlayerPawn::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartTouches.Remove(FingerIndex);
	CurrentTouches.Remove(FingerIndex);

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

	if (!HasSetInitialCameraTarget)
	{
		if (Camera)
		{
			SetNextTarget();
			Camera->StartDash();
			HasSetInitialCameraTarget = true;
		}
	}
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

void APlayerPawn::Hit()
{
	if (!HitIndicator) return;
	CurrentHitStage = HitIndicator->GetCurrentHitStage();


	HitIndicator->Destroy();
	HitIndicator = nullptr;

	CustomTimeDilation = 0.66f;

	if (CurrentHitStage == EHitStage::PERFECT)
	{
		if (CurrentTarget)
		{
			if (CurrentTarget->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
			{
				ICombatInterface* TempInterface = Cast<ICombatInterface>(CurrentTarget);
				if (!TempInterface) return;
				TempInterface->OnWeaponHit(this, "None");
			}
		}
	}



}

void APlayerPawn::OnDamageNotifyStarted()
{
	UWorld* World = GetWorld(); if (!World) return;
	if (!IsAttacking() || !CurrentWeapon || !CurrentTarget) return;
	
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

	HitIndicator = (AHitIndicator*)World->SpawnActor<AHitIndicator>(HitIndicatorClass, Transform);


}

void APlayerPawn::OnHitNotifyStarted()
{
	UWorld* World = GetWorld(); if (!World) return;

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Enemy Hit")));

	// TODO this will cause problems for multiple enemies
	CurrentWeapon->StopCheckingCollision();


	AttackStage = EAttackStage::HITTING;

	WeaponHitStartedRealTime = World->GetRealTimeSeconds();
	WeaponHitStartedTime = World->GetTimeSeconds();

	Camera->StartAttackingHit();

	//GetCharacterMovement()->MaxWalkSpeed = 0;
}

void APlayerPawn::OnDamageNotifyEnded()
{
	if (CurrentWeapon) CurrentWeapon->StopCheckingCollision();

	//StopAttackLoop();
	//CustomTimeDilation = 0.75;
}


const bool APlayerPawn::IsAttacking()
{
	if (ActionState == EActionState::ATTACKING) return true;

	return false;
	
}


void APlayerPawn::OnWeaponHitEnemy(AActor* Actor, FName Bone)
{
	
}


void APlayerPawn::DoLoops()
{
	if (ActionState == EActionState::DASHING)
	{
		DashingLoop();
		return;
	}
	else if (ActionState == EActionState::ATTACKING)
	{
		AttackingLoop();
		return;
	}
}

void APlayerPawn::StartDashing()
{
	UWorld* World = GetWorld(); if (!World) return;

	LastAttackedTarget = nullptr;
	SetNextTarget();
	ActionState = EActionState::DASHING;
	DashStartedRealTime = World->GetRealTimeSeconds();
	DashStartedTime = World->GetTimeSeconds();

	CustomTimeDilation = 1;
	GetCharacterMovement()->MaxWalkSpeed = 0;

	Camera->StartDash();
}

void APlayerPawn::DashingLoop()
{
	UWorld* World = GetWorld(); if (!World) return;

	if (!DashSpeedCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Trying to dash but DashSpeedCurve is null")));
		return;
	}

	UpdateDirection();

	const float DashRealTimePassed = World->GetRealTimeSeconds() - DashStartedRealTime;

	GetCharacterMovement()->MaxWalkSpeed = DashSpeedCurve->GetFloatValue(DashRealTimePassed) * RunSpeed;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s "), *FString::SanitizeFloat(GetCharacterMovement()->MaxWalkSpeed)));
	MoveTowardsDirection();
	SetActorRotation(LastDirection.Rotation().Quaternion());

	//
	if (IsAttacking()) return;
	if (!CurrentTarget) return;
	if (LastAttackedTarget == CurrentTarget) return;
	if (FVector::Distance(CurrentTarget->GetActorLocation(), GetActorLocation()) > AttackInfo.DistanceForSlowdown) return;
	StartAttacking();

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

	ActionState = EActionState::ATTACKING;
	AttackStage = EAttackStage::STARTING;

	StartAttackRealTime = World->GetRealTimeSeconds();
	StartAttackTime = World->GetTimeSeconds();

	float MontageDuration = PlayAnimMontage(AttackInfo.AttackMontage);

	Camera->StartAttacking();

	GetCharacterMovement()->MaxWalkSpeed = 0;

}

void APlayerPawn::AttackingLoop()
{
	UWorld* World = GetWorld(); if (!World) return;

	if (StartAttackRealTime < 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Start attack time < 0 but attacking loop is called. SHOULD NOT HAPPEN!!!")));
		return;
	}

	if (AttackStage == EAttackStage::HITTING)
	{
		float MinTime, MaxTime;
		HitSlowMoCurve->GetTimeRange(MinTime, MaxTime);
		const float WeaponHitTimePassed = World->GetRealTimeSeconds() - WeaponHitStartedRealTime;
		float TempSlowMoVal = HitSlowMoCurve->GetFloatValue(WeaponHitTimePassed);
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("%s "), *FString::SanitizeFloat(TempSlowMoVal)));
		CustomTimeDilation = TempSlowMoVal;

		if (WeaponHitTimePassed > MaxTime)
		{
			AttackStage = EAttackStage::ENDING;
			CustomTimeDilation = 1.f;
			Camera->StartEndingAttack();
		}
	}

	const float AttackTimePassed = World->GetTimeSeconds() - StartAttackTime;
	float MontageDuration = AttackInfo.AttackMontage->GetPlayLength() *
							(1 / AttackInfo.AttackMontage->RateScale) *
							(1 / CustomTimeDilation) *
							(1 / UGameplayStatics::GetGlobalTimeDilation(World));

	if (AttackTimePassed >= MontageDuration)
	{
		StartAttackRealTime = -1;
		StartAttackTime	    = -1;
		StopAttacking();
	}

}

void APlayerPawn::StopAttacking()
{
	AttackStage = EAttackStage::NONE;
	CurrentHitStage = EHitStage::NONE;
	OnAttackEnded();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Stop attack")));
	SetNextTarget();
	StartDashing();
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
	if (ActionState == EActionState::DASHING) return true;
	return false;
}

