// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/PlayerCam.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerPawn.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Camera/CameraShakeBase.h"

// Sets default values
APlayerCam::APlayerCam()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	//RootComponent = SceneComp;

	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	//sSpringArm->SetupAttachment(SceneComp);
	RootComponent = SpringArm;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);


}

// Called every frame
void APlayerCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
	DoLoops();

}

void APlayerCam::SetPlayer(APlayerPawn* Pawn)
{
	if (!Pawn || !SpringArm || !Camera)
	{
		Destroy();
		return;
	}
	SetActorRotation(DefaultRotation);

	Player = Pawn;
	StartingSpringArmLength = SpringArm->TargetArmLength;
	StartingSpringOffset = SpringArm->TargetOffset;
	//StartingSpringArmRot = SpringArm->GetComponentRotation();


	//const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget	, true);

	//AttachToActor(Player, Rules);

}


void APlayerCam::StartPreparingToAttack(bool IsForward)
{
	SpringArm->CameraLagSpeed = 30.f;
	SpringArm->CameraRotationLagSpeed = 30.f;
	if (IsForward)
	{
		CameraControlAccumulatedTime = 0;
	}
}

void APlayerCam::PreparingToAttackLoop()
{

	UWorld* World = GetWorld(); if (!World || !Player) return;
	if (!Player->AttackInfo.CameraZoomCurve || !Player->AttackInfo.CameraRotationCurve || !Player->AttackInfo.CameraLocationCurve) return;

	TArray<float> MaxTimes;
	float ZoomMinTime, ZoomMaxTime, RotMinTime, RotMaxTime, LocMinTime, LocMaxTime, MaxTime;
	float NewZoomVal;
	ZoomMinTime = ZoomMaxTime = RotMinTime = RotMinTime = RotMaxTime = LocMinTime = LocMaxTime = MaxTime = NewZoomVal = 0;

	FVector NewRotOffset = FVector::ZeroVector;
	FVector NewSocketOffset = FVector::ZeroVector;

	if (Player->AttackInfo.CameraZoomCurve)
	{
		Player->AttackInfo.CameraZoomCurve->GetTimeRange(ZoomMinTime, ZoomMaxTime);
		NewZoomVal = Player->AttackInfo.CameraZoomCurve->GetFloatValue(CameraControlAccumulatedTime);
	}
	if (Player->AttackInfo.CameraRotationCurve)
	{
		Player->AttackInfo.CameraRotationCurve->GetTimeRange(RotMinTime, RotMaxTime);
		NewRotOffset = Player->AttackInfo.CameraRotationCurve->GetVectorValue(CameraControlAccumulatedTime);
	}
	if (Player->AttackInfo.CameraLocationCurve)
	{
		Player->AttackInfo.CameraLocationCurve->GetTimeRange(LocMinTime, LocMaxTime);
		NewSocketOffset = Player->AttackInfo.CameraLocationCurve->GetVectorValue(CameraControlAccumulatedTime);
	}

	MaxTime = FMath::Max(ZoomMaxTime, RotMaxTime);
	MaxTime = FMath::Max(MaxTime, LocMaxTime);

	if (Player->ActionState == EActionState::PREPARING_TO_ATTACK)
	{
		// Is Current timeline finished forwards
		if (CameraControlAccumulatedTime > MaxTime)
		{
			return;
		}
		else
		{
			CameraControlAccumulatedTime += World->DeltaRealTimeSeconds;
		}
	}
	else if (Player->ActionState == EActionState::ENDING_ATTACK)
	{
		// Is Current timeline finished backwards
		if (CameraControlAccumulatedTime < 0)
		{
			return;
		}
		else
		{
			CameraControlAccumulatedTime -= World->DeltaRealTimeSeconds;
		}
	}
	else
	{
		return;
	}

	SpringArm->TargetArmLength = StartingSpringArmLength + NewZoomVal;
	SpringArm->TargetOffset = StartingSpringOffset + NewSocketOffset;

	FRotator Rot;
	Rot.Roll = NewRotOffset.X;
	Rot.Pitch = NewRotOffset.Y;
	Rot.Yaw = NewRotOffset.Z;

	const float Alpha = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxTime), FVector2D(0, 1), CameraControlAccumulatedTime);

	//TargetArm->SetRelativeRotation(FMath::Lerp(StartingSpringArmRot, Rot + StartingSpringArmRot, Alpha));

	//SpringArm->SetWorldRotation(Rot + StartingSpringArmRot);

	if (Player->ActionState == EActionState::PREPARING_TO_ATTACK || !Player->CurrentTarget)
	{
		SetActorRotation(Rot + DefaultRotation);
	}
	else
	{
		FRotator LookAtTargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->CurrentTarget->GetActorLocation());
		FRotator TempRot = Rot + DefaultRotation;

		FRotator FinalRot = FMath::Lerp(TempRot, SpringArm->GetComponentRotation(), Alpha);
		FinalRot.Yaw = TempRot.Yaw;

		SetActorRotation(FinalRot);
	}


	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%s"), *FString::SanitizeFloat(World->DeltaRealTimeSeconds)));
}

void APlayerCam::StartAttacking()
{
}

void APlayerCam::AttackingLoop()
{
}

void APlayerCam::StartAttackingHit()
{
	PlayCameraShake(CameraShakeOnEnemyHitWeak);
	
}

void APlayerCam::AttackingHitLoop()
{
}

void APlayerCam::StartEndingAttack()
{
	StartPreparingToAttack(false);
}

void APlayerCam::EndingAttackLoop()
{
}

void APlayerCam::StartRunning()
{
	SpringArm->CameraLagSpeed = 6.f;
	SpringArm->CameraRotationLagSpeed = 6.f;
}

void APlayerCam::PlayCameraShake(TSoftClassPtr<UCameraShakeBase> CameraShakeClass)
{
	UWorld* World = GetWorld(); if (!World) return;
	if (!CameraShakeClass) return;
	CameraShakeClass.LoadSynchronous();
	UClass* CamShakeClass = CameraShakeClass.Get();
	if (!CamShakeClass) return;
	// TODO maybe orient towards slash?
	World->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(World, CamShakeClass, GetActorLocation(),3000,3000,1,0 );

}

// Default State
void APlayerCam::RunningLoop()
{
}

void APlayerCam::StartDash()
{
}

void APlayerCam::DashingLoop()
{
}


void APlayerCam::StartStartingDash()
{
}

void APlayerCam::StartingDashLoop()
{
}


void APlayerCam::DoLoops()
{
	if (Player->ActionState == EActionState::STARTING_DASH)
	{
		StartingDashLoop();
		return;
	}
	else if (Player->ActionState == EActionState::DASHING)
	{
		DashingLoop();
		return;
	}
	else if (Player->ActionState == EActionState::PREPARING_TO_ATTACK)
	{
		PreparingToAttackLoop();
		return;
	}
	else if (Player->ActionState == EActionState::ATTACKING)
	{
		AttackingLoop();
		return;
	}
	else if (Player->ActionState == EActionState::ATTACKING_HIT)
	{
		AttackingHitLoop();
		return;
	}
	else if (Player->ActionState == EActionState::ENDING_ATTACK)
	{
		EndingAttackLoop();
		return;
	}
	else if (Player->ActionState == EActionState::RUNNING)
	{
		RunningLoop();
		return;
	}

}

void APlayerCam::Move()
{
	if (!Player) return;
	FVector NewLoc = Player->GetActorLocation() + DefaultOffset;
	SetActorLocation(NewLoc);

	//FRotator Rot = GetActorRotation();
	//Rot.Yaw = Player->GetActorRotation().Yaw;
	//SetActorRotation(Player->GetActorRotation());
}
