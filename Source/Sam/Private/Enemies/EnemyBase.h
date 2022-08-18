// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/CombatInterface.h"
#include "EnemyBase.generated.h"

UCLASS()
class AEnemyBase : public AActor, public ICombatInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCapsuleComponent* Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* SkelMesh;

	class UEnemySpawnerComponent* Spawner;

	UFUNCTION(BlueprintCallable)
	void Die();

	void Init();

	FTimerHandle ReleaseToPoolTH;

	void ReleaseToPool();

	bool IsAlive = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RotateTowardsPlayer();

	void ActivateRagdoll();

	virtual void OnWeaponHit(AActor* Actor, FName Bone) override;

	class APawn* PlayerPawn;

	void UpdatePlayerPawn();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
