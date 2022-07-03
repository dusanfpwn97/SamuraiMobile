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


APlayerPawn::APlayerPawn()
{
	//CapsuleSiz
	GetCapsuleComponent()->SetCapsuleSize(42, 96, false);
	RootComponent = GetCapsuleComponent();
	
	TargetArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	TargetArm->SetupAttachment(GetMesh());
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
	SetActorRotation(LastDirection.Rotation());
}

void APlayerPawn::UpdateSpeed()
{
	UWorld* World = GetWorld();
	if (!CurrentTarget || !World) return;

	if (ActionState == EActionState::RUNNING)
	{
		if (FVector::Distance(CurrentTarget->GetActorLocation(), GetActorLocation()) > AttackInfo.DistanceForSlowdown)
		{
			CurrentSpeed = RunSpeed;
			return;
		}
		else
		{
			CurrentSpeed = PrepareToAttackSpeed;
			return;
		}
	}
	else if (ActionState == EActionState::DASHING)
	{
		if (!DashSpeedCurve)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Trying to dash but StartDashSpeedCurve is null")));
			return;
		}
		else if(FVector::Distance(CurrentTarget->GetActorLocation(), GetActorLocation()) < AttackInfo.DistanceForSlowdown)
		{
			CurrentSpeed = PrepareToAttackSpeed;
			return;
		}
		else
		{
			CurrentSpeed = DashSpeedCurve->GetFloatValue(DashDeltaAccumulated) * RunSpeed;
			DashDeltaAccumulated += World->GetDeltaSeconds();
		}
	}
	else if (ActionState == EActionState::STARTING_DASH)
	{
		if (!AttackInfo.StartDashSpeedCurve)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Trying to dash but StartDashSpeedCurve is null")));
			return;
		}
		else
		{
			CurrentSpeed = AttackInfo.StartDashSpeedCurve->GetFloatValue(DashDeltaAccumulated) * RunSpeed;
			DashDeltaAccumulated += World->GetDeltaSeconds();
		}
	}
	else if(ActionState == EActionState::ATTACKING || ActionState == EActionState::NOT_MOVING)
	{
		CurrentSpeed = 0;
		return;
	}
	else if(ActionState == EActionState::PREPARING_FOR_ATTACK)
	{
		CurrentSpeed = PrepareToAttackSpeed;
		return;
	}

}

void APlayerPawn::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{

	StartDashing();
}

void APlayerPawn::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StartAttacking();
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpeed();

	MoveTowardsTarget();

	UpdateRotation();


}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	ActionState = EActionState::RUNNING;
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
}



void APlayerPawn::StartDashing()
{
	UWorld* World = GetWorld();
	if (!AttackInfo.StartDashSpeedCurve || !World || !AttackInfo.StartDashMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Dash cannot be started. Something is null. PlayerPawn->StartDashing()")));
		return;
	}

	World->GetTimerManager().ClearTimer(ScheduleNextActionTH);

	ActionState = EActionState::STARTING_DASH;
	DashDeltaAccumulated = 0.f;

	float MontageDuration = PlayAnimMontage(AttackInfo.StartDashMontage);

	float MinTime, MaxTime;
	AttackInfo.StartDashSpeedCurve->GetTimeRange(MinTime, MaxTime);

	World->GetTimerManager().SetTimer(ScheduleNextActionTH, this, &APlayerPawn::ContinueDashing, MaxTime, false);

}

void APlayerPawn::ContinueDashing()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(ScheduleNextActionTH);
	ActionState = EActionState::DASHING;
	DashDeltaAccumulated = 0.f;
}

void APlayerPawn::OnDamageNotifyStarted()
{
	if (ActionState != EActionState::ATTACKING || !CurrentWeapon || !CurrentTarget) return;
	
	AEnemyBase* Enemy = (AEnemyBase*)CurrentTarget;
	if (!Enemy) return;
	USkeletalMeshComponent * SKMComp = Enemy->SkelMesh;
	if (!SKMComp) return;

	CurrentWeapon->StartCheckingCollision();

}

void APlayerPawn::OnDamageNotifyEnded()
{
	CurrentWeapon->StopCheckingCollision();
}

void APlayerPawn::OnWeaponCollided(AActor* Actor, FName Bone)
{
}

void APlayerPawn::StartAttacking()
{
	UWorld* World = GetWorld();
	if (!World || !AttackInfo.AttackMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Attack cannot be started. Something is null. PlayerPawn->StartAttacking()")));
		return;
	}

	//AttackInfo.AttackMontage->noti

	World->GetTimerManager().ClearTimer(ScheduleNextActionTH);
	ActionState = EActionState::ATTACKING;

	float MontageDuration = PlayAnimMontage(AttackInfo.AttackMontage);

	World->GetTimerManager().SetTimer(ScheduleNextActionTH, this, &APlayerPawn::StartRunning, MontageDuration, false);

}

void APlayerPawn::StartRunning()
{


	ActionState = EActionState::RUNNING;
}
