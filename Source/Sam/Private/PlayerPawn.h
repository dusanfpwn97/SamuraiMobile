// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Misc/CombatInterface.h"
#include "Misc/HitIndicator.h"

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
	ATTACKING
};

UENUM(BlueprintType)
enum class EAttackStage : uint8
{
	NONE,
	STARTING,
	HITTING,
	ENDING
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
		void OnHitNotifyStarted();
	UFUNCTION(BlueprintCallable)
	void OnDamageNotifyEnded();
	UFUNCTION(BlueprintCallable)
	const bool IsAttacking();
	UFUNCTION(BlueprintCallable)
	const bool IsDashing();

	bool GetTargetUnderFinger(ETouchIndex::Type FingerIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		TArray<FAttackInfo> AttackInfos;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
		FAttackInfo AttackInfo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
		int32 AttackInfoIndexOverride = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		AActor* CurrentTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		EActionState ActionState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
		EAttackStage AttackStage;

protected:
	void PickRandomAttackInfo();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APlayerCam* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<APlayerCam> CameraSoftClass;

	void SpawnCamera();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnCameraCreated();

	bool HasSetInitialCameraTarget = false;

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
	TSubclassOf<class AHitIndicator> HitIndicatorClass;
	AActor* LastAttackedTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool IsInDamageWindow = false;

	float DashStartedRealTime = -1;
	float DashStartedTime = -1;
	float StartAttackRealTime = -1;
	float StartAttackTime = -1;
	float WeaponHitStartedRealTime = -1;
	float WeaponHitStartedTime = -1;
	

	AHitIndicator* HitIndicator;
	EHitStage CurrentHitStage;

	//FTimerHandle ScheduleHitTH;

	//
	void SetStartingValues();
	void MoveTowardsDirection();
	void SetNextTarget();
	void Hit();


	UFUNCTION(BlueprintImplementableEvent)
	void OnAttackEnded();

	virtual void OnWeaponHitEnemy(AActor* Actor, FName Bone) override;

	void AdvanceAttackTime();

	void DoLoops();

	void StartDashing();
	void DashingLoop();

	void StartAttacking();
	void AttackingLoop();
	void StopAttacking();

	void UpdateDirection();


	bool StartedAttackingGame = false;
};

