// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "HitIndicator.generated.h"


class UCurveFloat;

UENUM(BlueprintType)
enum class EHitStage : uint8
{
	MISS,
	NEUTRAL,
	GOOD,
	PERFECT
};


UCLASS()
class AHitIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHitIndicator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Goal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Moving;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float GetDistance();
	EHitStage GetCurrentHitStage();

	FVector MovingStartingLoc;

	void SetInitialPositions();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		TArray<UCurveFloat*> MovementCurves;

	UCurveFloat* MovementCurveToUse;
	float MoveStartRealTime;

	void Move();

	void Rotate();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
