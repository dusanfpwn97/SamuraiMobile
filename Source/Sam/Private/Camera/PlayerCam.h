// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerCam.generated.h"

class UCurveFloat;
class UCurveVector;
class UCameraShakeBase;

UCLASS()
class APlayerCam : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerCam();
	virtual void Tick(float DeltaTime) override;

	void SetPlayer(class APlayerPawn* Pawn);

	void StartStartingDash();
	void StartDash();
	void StartPreparingToAttack(bool IsForward);
	void StartAttacking();
	void StartAttackingHit();
	void StartEndingAttack();
	void StartRunning();

	void PlayCameraShake(TSoftClassPtr<UCameraShakeBase> CameraShakeClass);

	//void StopCameraAttackMovement();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TSoftClassPtr<UCameraShakeBase> CameraShakeOnEnemyHitWeak;

	void DoLoops();

	void RunningLoop(); // Default State
	void EndingAttackLoop();
	void AttackingHitLoop();
	void AttackingLoop();
	void StartingDashLoop();
	void PreparingToAttackLoop();
	void DashingLoop();


	//UPROPERTY(EditAnywhere)
	//	class USceneComponent* SceneComp;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//	class USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APlayerPawn* Player;

	// Runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		float StartingSpringArmLength;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		FVector StartingSpringOffset;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	//	FRotator StartingSpringArmRot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	FRotator DefaultRotation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	FVector DefaultOffset;

	FRotator GetRunningRotation();

	float CameraControlAccumulatedTime = -1;

	void Move();




};
