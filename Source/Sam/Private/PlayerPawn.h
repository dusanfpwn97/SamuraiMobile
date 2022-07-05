// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Misc/CombatInterface.h"
#include "PlayerPawn.generated.h"

class ABaseWeapon;
class UAnimMontage;
class UCurveFloat;

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

USTRUCT(BlueprintType)
struct FAttackInfo
{
	GENERATED_BODY()

		//~ The following member variable will be accessible by Blueprint Graphs:
		// This is the tooltip for our test variable.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float DistanceForSlowdown = 280.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* StartDashMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UCurveFloat* StartDashSpeedCurve;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool IsInDamageWindow = false;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return TargetArm; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
		void OnDamageNotifyStarted();
	UFUNCTION(BlueprintCallable)
		void OnDamageNotifyEnded();
	UFUNCTION(BlueprintCallable)
		bool IsAttacking();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USpringArmComponent* TargetArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCameraComponent* FollowCamera;

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* CurrentTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ABaseWeapon* CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<class ASlashIndicator> SlashIndicatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		UCurveFloat* DashSpeedCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		UCurveFloat* HitSlowMoCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		float RunSpeed = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
		float PrepareToAttackSpeed = 50.f;


	void HitSlowMoLoop();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float CurrentSpeed = 700.f;

	UFUNCTION(BlueprintCallable)
	void MoveTowardsTarget();
	UFUNCTION(BlueprintCallable)
	void SetNextTarget();

	void UpdateRotation();
	void UpdateSpeed();
	void StartDashing();
	void StartAttacking();
	void StartRunning();
	void ContinueDashing();
	void PrepareForAttack();

	void CheckActionStates();

	AActor* PrepareForAttackTarget;

	FTimerHandle ScheduleNextActionTH;

	virtual void OnWeaponHitEnemy(AActor* Actor, FName Bone) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LastDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EActionState ActionState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FAttackInfo AttackInfo;

	float DashDeltaAccumulated;
	float AttackSlowMoDeltaAccumulated;
	float CurrentHitSlowMoTimeAccumulated = -1;
	float StartRunningAccumulated = -1;
	void CheckStartRunningAfterAttack();
	


};

