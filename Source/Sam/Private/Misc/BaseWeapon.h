// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/CombatInterface.h"
#include "BaseWeapon.generated.h"

class USceneComponent;


UCLASS(Abstract)
class ABaseWeapon : public AActor, public ICombatInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	void Activate();
	void Deactvate();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* CollisionPoint6;

	TArray<USceneComponent*> CollisionPoints;

	TArray<AActor*> CollidedActors;

	bool ShouldCheck;

	

public:	

	UFUNCTION(BlueprintCallable)
	float GetClosestCollisionDistance(AActor* Actor);

	void StartCheckingCollision();
	void CheckCollision();
	void StopCheckingCollision();

	virtual void Tick(float DeltaTime) override;

	FTimerHandle CheckCollitionTH;

	AActor* Owner;
};
