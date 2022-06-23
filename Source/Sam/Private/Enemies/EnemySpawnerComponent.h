// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Pool/PoolManager.h"
#include "EnemySpawnerComponent.generated.h"

class AEnemyBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract, Blueprintable)
class UEnemySpawnerComponent : public UPoolManager
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemySpawnerComponent();


	TArray<AEnemyBase*> SpawnedEnemies;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AEnemyBase> ClassToSpawn;

	FTimerHandle EnemySpawnTH;

	void SpawnEnemy();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
