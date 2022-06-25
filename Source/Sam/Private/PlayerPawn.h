// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "PlayerPawn.generated.h"

UCLASS(config=Game)
class APlayerPawn : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCapsuleComponent* Capsule;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* SKMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Weapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USpringArmComponent* TargetArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* CurrentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BaseSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CurrentSpeed = 700.f;

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	void MoveTowardsTarget();
	void SetNextTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector LastDirection;

	void UpdateRotation();
public:

	APlayerPawn();
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return TargetArm; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

