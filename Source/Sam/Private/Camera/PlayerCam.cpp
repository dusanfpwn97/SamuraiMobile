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
	//
	//
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	RootComponent = SpringArm;
	//SpringArm->SetupAttachment(SceneComp);
	//
	//SpringArm->bEnableCameraLag = true;
	//SpringArm->bEnableCameraRotationLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called every frame
void APlayerCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DebugPrintState();
	Move();
	DoLoops();


	ApplyRotation();
}

void APlayerCam::SetPlayer(APlayerPawn* Pawn)
{
	if (!Pawn || !Camera)
	{
		Destroy();
		return;
	}
	Camera->SetWorldRotation(GetRunningRotation());

	Player = Pawn;
	StartingSpringArmLength = SpringArm->TargetArmLength;
	StartingSpringOffset = SpringArm->SocketOffset;
	
}


void APlayerCam::StartPreparingToAttack(bool IsForward)
{

	//SpringArm->CameraLagSpeed = 40.f;
	//SpringArm->CameraRotationLagSpeed = 40.f;
	if (IsForward)
	{
		CameraControlAccumulatedTime = 0;
	}
}

void APlayerCam::PreparingToAttackLoop()
{

	//UWorld* World = GetWorld(); if (!World || !Player) return;
	//if (!Player->AttackInfo.CameraZoomCurve || !Player->AttackInfo.CameraRotationCurve || !Player->AttackInfo.CameraLocationCurve) return;
	//
	//TArray<float> MaxTimes;
	//float ZoomMinTime, ZoomMaxTime, RotMinTime, RotMaxTime, LocMinTime, LocMaxTime, MaxTime;
	//float NewZoomVal;
	//ZoomMinTime = ZoomMaxTime = RotMinTime = RotMinTime = RotMaxTime = LocMinTime = LocMaxTime = MaxTime = NewZoomVal = 0;
	//
	//FVector NewRotOffset = FVector::ZeroVector;
	//FVector NewSocketOffset = FVector::ZeroVector;
	//
	//if (Player->AttackInfo.CameraZoomCurve)
	//{
	//	Player->AttackInfo.CameraZoomCurve->GetTimeRange(ZoomMinTime, ZoomMaxTime);
	//	NewZoomVal = Player->AttackInfo.CameraZoomCurve->GetFloatValue(CameraControlAccumulatedTime);
	//}
	//if (Player->AttackInfo.CameraRotationCurve)
	//{
	//	Player->AttackInfo.CameraRotationCurve->GetTimeRange(RotMinTime, RotMaxTime);
	//	NewRotOffset = Player->AttackInfo.CameraRotationCurve->GetVectorValue(CameraControlAccumulatedTime);
	//}
	//if (Player->AttackInfo.CameraLocationCurve)
	//{
	//	Player->AttackInfo.CameraLocationCurve->GetTimeRange(LocMinTime, LocMaxTime);
	//	NewSocketOffset = Player->AttackInfo.CameraLocationCurve->GetVectorValue(CameraControlAccumulatedTime);
	//}
	//
	//MaxTime = FMath::Max(ZoomMaxTime, RotMaxTime);
	//MaxTime = FMath::Max(MaxTime, LocMaxTime);
	//
	//if (Player->ActionState == EActionState::PREPARING_TO_ATTACK)
	//{
	//	// Is Current timeline finished forwards
	//	if (CameraControlAccumulatedTime > MaxTime)
	//	{
	//		return;
	//	}
	//	else
	//	{
	//		CameraControlAccumulatedTime += World->DeltaRealTimeSeconds;
	//	}
	//}
	//else if (Player->ActionState == EActionState::ENDING_ATTACK)
	//{
	//	// Is Current timeline finished backwards
	//	if (CameraControlAccumulatedTime < 0)
	//	{
	//		return;
	//	}
	//	else
	//	{
	//		CameraControlAccumulatedTime -= World->DeltaRealTimeSeconds;
	//	}
	//}
	//else
	//{
	//	return;
	//}
	//
	//SpringArm->TargetArmLength = StartingSpringArmLength + NewZoomVal;
	//SpringArm->SocketOffset = StartingSpringOffset + NewSocketOffset;
	//
	//FRotator Rot;
	//Rot.Roll = NewRotOffset.X;
	//Rot.Pitch = NewRotOffset.Y;
	//Rot.Yaw = NewRotOffset.Z;
	//
	//const float Alpha = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxTime), FVector2D(0, 1), CameraControlAccumulatedTime);
	//
	//if (Player->ActionState == EActionState::PREPARING_TO_ATTACK || !Player->CurrentTarget)
	//{
	//	NewTargetRot = Rot + GetRunningRotation();
	//	return;
	//}
	//
	//if (Player->ActionState == EActionState::ENDING_ATTACK)
	//{
	//	if (!Player->CurrentTarget) return;
	//	FRotator LookAtTargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->CurrentTarget->GetActorLocation());
	//	FRotator TempRot = Rot + GetRunningRotation();
	//
	//	FRotator FinalRot = FMath::Lerp(TempRot, GetActorRotation(), Alpha);
	//	FinalRot.Yaw = TempRot.Yaw;
	//
	//	NewTargetRot = FinalRot;
	//
	//	return;
	//}


	



	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%s"), *FString::SanitizeFloat(NewZoomVal)));
}

void APlayerCam::StartAttacking()
{
	CameraControlAccumulatedTime = 0.f;
}

void APlayerCam::AttackingLoop()
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

	if (Player->ActionState == EActionState::ATTACKING)
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
	else if (Player->AttackStage == EAttackStage::ENDING)
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
	SpringArm->SocketOffset = StartingSpringOffset + NewSocketOffset;

	FRotator Rot;
	Rot.Roll = NewRotOffset.X;
	Rot.Pitch = NewRotOffset.Y;
	Rot.Yaw = NewRotOffset.Z;

	const float Alpha = FMath::GetMappedRangeValueClamped(FVector2D(0, MaxTime), FVector2D(0, 1), CameraControlAccumulatedTime);

	if (Player->AttackStage == EAttackStage::STARTING || Player->AttackStage == EAttackStage::HITTING || !Player->CurrentTarget)
	{
		NewTargetRot = Rot + GetRunningRotation();
		return;
	}

	if (Player->AttackStage == EAttackStage::ENDING)
	{
		if (!Player->CurrentTarget) return;
		FRotator LookAtTargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Player->CurrentTarget->GetActorLocation());
		FRotator TempRot = Rot + GetRunningRotation();

		FRotator FinalRot = FMath::Lerp(TempRot, GetActorRotation(), Alpha);
		FinalRot.Yaw = TempRot.Yaw;

		NewTargetRot = FinalRot;

		return;
	}
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
	//StartPreparingToAttack(false);
}

void APlayerCam::EndingAttackLoop()
{
	UWorld* World = GetWorld(); if (!World) return;

	MoveToDefaultLoop();
}

void APlayerCam::PreparingToDashLoop()
{
	MoveToDefaultLoop();
}

void APlayerCam::StartDash()
{

}

void APlayerCam::DashingLoop()
{
	UWorld* World = GetWorld(); if (!World) return;
	
	MoveToDefaultLoop();
}

void APlayerCam::StartingDashLoop()
{

}

void APlayerCam::DoLoops()
{
	if (Player->ActionState == EActionState::DASHING)
	{
		DashingLoop();
		return;
	}
	//else if (Player->ActionState == EActionState::PREPARING_TO_ATTACK)
	//{
	//	PreparingToAttackLoop();
	//}
	else if (Player->ActionState == EActionState::ATTACKING)
	{
		AttackingLoop();
		return;
	}
	else if (Player->AttackStage == EAttackStage::HITTING)
	{
		AttackingLoop();
		AttackingHitLoop();
		return;
	}
	else if (Player->AttackStage == EAttackStage::ENDING)
	{
		AttackingLoop();
		EndingAttackLoop();
		return;
	}

}

FRotator APlayerCam::GetRunningRotation()
{
	if (!Player) return FRotator();

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, FString::Printf(TEXT("%s"), *Player->GetActorRotation().ToString()));

	return Player->GetActorRotation() + DefaultRotation;

}

void APlayerCam::Move()
{
	if (!Player) return;
	UWorld* World = GetWorld(); if (!World) return;
	
	//SetActorLocation(FMath::VInterpTo(GetActorLocation(), Player->GetActorLocation() + DefaultOffset, World->GetDeltaSeconds(), 13.f));
	SetActorLocation(Player->GetActorLocation() + FVector(0,0, DefaultZOffset));
	
}

void APlayerCam::ApplyRotation()
{
	UWorld* World = GetWorld(); if (!World) return;

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewTargetRot, World->DeltaTimeSeconds, 9));
}

void APlayerCam::MoveToDefaultLoop()
{
	UWorld* World = GetWorld(); if (!World) return;
	NewTargetRot = GetRunningRotation();
	SpringArm->SocketOffset	   = FMath::VInterpTo(SpringArm->SocketOffset, StartingSpringOffset, World->GetDeltaSeconds(), 7.f);
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, StartingSpringArmLength, World->GetDeltaSeconds(), 7.f);
}

void APlayerCam::PlayCameraShake(TSoftClassPtr<UCameraShakeBase> CameraShakeClass)
{
	UWorld* World = GetWorld(); if (!World) return;
	if (CameraShakeClass.IsNull()) return;

	CameraShakeClass.LoadSynchronous();
	UClass* CamShakeClass = CameraShakeClass.Get();
	if (!CamShakeClass) return;

	// TODO maybe orient towards slash?
	World->GetFirstPlayerController()->PlayerCameraManager->PlayWorldCameraShake(World, CamShakeClass, GetActorLocation(), 3000, 3000, 1, false);

}