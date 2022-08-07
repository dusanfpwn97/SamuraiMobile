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
	DASHING,
	PREPARING_TO_ATTACK,
	STARTING_ATTACK,
	ATTACKING_HIT,
	ENDING_ATTACK
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
	UCurveFloat* StartAttackSlomoCurve;
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

	UFUNCTION(BlueprintCallable)
	void OnDamageNotifyStarted();
	UFUNCTION(BlueprintCallable)
	void OnDamageNotifyEnded();
	UFUNCTION(BlueprintCallable)
	const bool IsAttacking();
	UFUNCTION(BlueprintCallable)
	const bool IsDashing();

	bool GetTargetUnderFinger(ETouchIndex::Type FingerIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
		FAttackInfo AttackInfo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		AActor* CurrentTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		EActionState ActionState;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APlayerCam* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<APlayerCam> CameraSoftClass;

	void SpawnCamera();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnCameraCreated();

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpdateSwipeDirection();

	FVector SwipeDirection;
	TMap<ETouchIndex::Type, FVector> StartTouches;
	TMap<ETouchIndex::Type, FVector> CurrentTouches;

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
	FVector LastDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	ABaseWeapon* CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
	TSubclassOf<class ASlashIndicator> SlashIndicatorClass;
	AActor* LastAttackedTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool IsInDamageWindow = false;

	float DashAccumulatedTime = -1;
	float CurrentHitSlowMoAccumulatedTime = -1;
	float AttackAccumulatedTime = -1;

	ASlashIndicator* SlashIndicator;
	//
	void SetStartingValues();
	void MoveTowardsDirection();
	void SetNextTarget();
	void TryToSlash();


	UFUNCTION(BlueprintImplementableEvent)
	void OnAttackEnded();

	virtual void OnWeaponHitEnemy(AActor* Actor, FName Bone) override;

	void AdvanceAttackTime();

	void DoLoops();

	void StartDashing();
	void DashingLoop();

	void StartPreparingToAttack();
	void PreparingToAttackLoop();

	void StartAttacking();
	void AttackingLoop();
	void StopAttackLoop();

	void StartAttackingHit();
	void AttackingHitLoop();

	void StartEndingAttack();
	void EndingAttackLoop();
	
	void UpdateDirection();


	bool StartedAttackingGame = false;
};

