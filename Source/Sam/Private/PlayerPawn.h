// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Misc/CombatInterface.h"
#include "PlayerPawn.generated.h"

class ABaseWeapon;
class UAnimMontage;
class UCurveFloat;
class UCurveVector;

UENUM(BlueprintType)
enum class EActionState : uint8
{
	NOT_MOVING,
	STARTING_DASH,
	DASHING,
	PREPARING_TO_ATTACK,
	ATTACKING,
	ATTACKING_HIT,
	RUNNING
};

UENUM(BlueprintType)
enum class ECameraMovement : uint8
{
	NOT_MOVING,
	FORWARD,
	BACKWARD
};

USTRUCT(BlueprintType)
struct FAttackInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DistanceForSlowdown = 280.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* StartDashMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* StartDashSpeedCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* CameraZoomCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveVector* CameraLocationCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveVector* CameraRotationCurve;

};

UCLASS(config=Game)
class APlayerPawn : public ACharacter, public ICombatInterface
{
	GENERATED_BODY()

public:

	APlayerPawn();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void EquipNewWeapon(TSoftClassPtr<ABaseWeapon> WepClass);


	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return TargetArm; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void OnDamageNotifyStarted();
	UFUNCTION(BlueprintCallable)
	void OnDamageNotifyEnded();
	UFUNCTION(BlueprintCallable)
	bool IsAttacking();
	UFUNCTION(BlueprintCallable)
	bool IsDashing();


protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USpringArmComponent* TargetArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCameraComponent* FollowCamera;

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// Setup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	UCurveFloat* DashSpeedCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	UCurveFloat* HitSlowMoCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float RunSpeed = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	float PrepareToAttackSpeed = 50.f;

	// Runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	float StartingSpringArmLength;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FVector StartingSpringOffset;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FRotator StartingSpringArmRot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	float CurrentSpeed = 700.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FVector LastDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	EActionState ActionState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
	FAttackInfo AttackInfo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	AActor* CurrentTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	ABaseWeapon* CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
	TSubclassOf<class ASlashIndicator> SlashIndicatorClass;
	AActor* PrepareForAttackTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool IsInDamageWindow = false;

	float DashAccumulatedTime = -1;
	float CurrentHitSlowMoAccumulatedTime = -1;
	float AttackAccumulatedTime = -1;
	float CameraControlAccumulatedTime = -1;

	//
	void SetStartingValues();
	void MoveTowardsTarget();
	void SetNextTarget();
	void UpdateRotation();
	void CheckPrepareToAttack();


	UFUNCTION(BlueprintImplementableEvent)
	void OnAttackEnded();

	virtual void OnWeaponHitEnemy(AActor* Actor, FName Bone) override;


	ECameraMovement CameraMovement;
	void CameraAttackMovementLoop();
	//UFUNCTION(BlueprintImplementableEvent)
	void StartCameraAttackMovement(ECameraMovement CamMovement);
	void StopCameraAttackMovement();

	void DoLoops();
	void StartStartingDash();
	void StartingDashLoop();
	//void StopStartingToDash();

	void StartDashing();
	void DashingLoop();
	//void StopDashing();

	void StartPreparingToAttack();
	void PreparingToAttackLoop();
	//void StopPreparingToAttack();

	void StartAttacking(bool Reset);
	void AttackingLoop();
	void StopAttack();
	void AdvanceAttackTime();

	void StartAttackingHit();
	void AttackingHitLoop();
	
	void StartRunning();
	void RunningLoop();

};

