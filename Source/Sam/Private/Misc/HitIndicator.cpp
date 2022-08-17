// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/HitIndicator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"


// Sets default values
AHitIndicator::AHitIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	Goal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Goal"));
	Goal->SetupAttachment(Root);
	Goal->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Goal->SetGenerateOverlapEvents(false);
	//Goal->SetRelativeRotation(FRotator(-90, 0, 0));

	Moving = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Moving"));
	Moving->SetupAttachment(Root);
	Moving->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Moving->SetGenerateOverlapEvents(false);
	//Moving->SetRelativeRotation(FRotator(-90, 0, 0));
}

// Called when the game starts or when spawned
void AHitIndicator::BeginPlay()
{
	Super::BeginPlay();
	
	SetInitialPositions();
}

float AHitIndicator::GetDistance()
{
	return FVector::Dist(Goal->GetComponentLocation(), Moving->GetComponentLocation());
}

EHitStage AHitIndicator::GetCurrentHitStage()
{
	
	return EHitStage::PERFECT;
}

void AHitIndicator::SetInitialPositions()
{
	UWorld* World = GetWorld(); if (!World) return;

	Moving->SetWorldLocation(Goal->GetComponentLocation());
	//Moving->SetRelativeLocation
	
	float Rand = FMath::FRand();
	float YRand = 0.f;
	float ZRand = 0.f;

	if (Rand > 0.5)
	{
		YRand = FMath::FRandRange(40.f, 60.f);
		ZRand = FMath::FRandRange(40.f, 60.f);
	}
	else
	{
		YRand = FMath::FRandRange(-40.f, -60.f);
		ZRand = FMath::FRandRange(-40.f, -60.f);
	}
	
	
	Moving->SetRelativeLocation(FVector(0.f, YRand, ZRand));
	MovingStartingLoc = Moving->GetComponentLocation();

	// Use random movement curve
	MovementCurveToUse = MovementCurves[FMath::FloorToInt32(FMath::FRand() * MovementCurves.Num())];


	MoveStartRealTime = World->GetRealTimeSeconds();
}

void AHitIndicator::Move()
{
	UWorld* World = GetWorld(); if (!World) return;

	float MinTime, MaxTime;
	MovementCurveToUse->GetTimeRange(MinTime, MaxTime);
	const float MoveStartRealTimePassed = World->GetRealTimeSeconds() - MoveStartRealTime;
	float MoveVal = MovementCurveToUse->GetFloatValue(MoveStartRealTimePassed);


	FVector NewLoc = FMath::Lerp(MovingStartingLoc, Goal->GetComponentLocation(), MoveVal);

	Moving->SetWorldLocation(NewLoc);

}

void AHitIndicator::Rotate()
{
	UWorld* World = GetWorld(); if (!World) return;

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0);

	if (!CameraManager) return;

	FRotator LookAtCameraRot = (Goal->GetComponentLocation() - CameraManager->GetCameraLocation()).Rotation();
	LookAtCameraRot.Yaw += 90.f;
	LookAtCameraRot.Roll += 90.f;
	
	Goal->SetWorldRotation(LookAtCameraRot);
	Moving->SetWorldRotation(LookAtCameraRot);
}

// Called every frame
void AHitIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
	Rotate();
}

