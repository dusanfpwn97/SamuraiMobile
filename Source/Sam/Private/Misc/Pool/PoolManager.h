// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoolManager.generated.h"

USTRUCT()
struct FPooledActors
{
	GENERATED_BODY()

	TArray<AActor*> Actors;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPoolManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPoolManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	AActor* SpawnActorFromPool(UClass* ActorClass);

	void ReleaseToPool(AActor* Actor);

	UPROPERTY()
		TMap<UClass*, FPooledActors> PooledActorsMap;

	UPROPERTY()
		TArray<AActor*> AllSpawnedActors;
private:

};
