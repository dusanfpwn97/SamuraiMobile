// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemySpawnerComponent.h"
#include "Enemies/EnemyBase.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/HelperLibrary.h"



// Sets default values for this component's properties
UEnemySpawnerComponent::UEnemySpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}


// Called when the game starts
void UEnemySpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	ClassToSpawn.LoadSynchronous();

	if (ClassToSpawn.IsValid())
	{
		World->GetTimerManager().SetTimer(EnemySpawnTH, this, &UEnemySpawnerComponent::SpawnEnemy, 0.2, true);
	}
}

void UEnemySpawnerComponent::SpawnEnemy()
{

	UWorld* World = GetWorld();
	if (!World || !ClassToSpawn.IsValid()) return;

	if (SpawnedEnemies.Num() >= 20)
	{
		World->GetTimerManager().ClearTimer(EnemySpawnTH);
	}

	//float Y = FMath::FRandRange(-700.f, 700.f); // left right
	//float X = SpawnedEnemies.Num() * FMath::FRandRange(1000.f, 1300.f) + 800.f; // Distance apart
	//FVector Loc = FVector(X, Y, 86.f);
	UClass* TempClass = ClassToSpawn.Get();
	if (!TempClass) return;

	FVector Loc = UHelperLibrary::GetRandomPointInCircle(FVector::ZeroVector, SpawnedEnemies.Num() * 200 + 1200);
	Loc.Z = 86.f;
	FRotator Rot = FRotator(0, 180, 0);

	AEnemyBase* SpawnedActor = (AEnemyBase*) SpawnActorFromPool(TempClass);
	
	if (SpawnedActor)
	{
		SpawnedActor->SetActorLocation(Loc);
		SpawnedActor->SetActorRotation(Rot);
		SpawnedActor->Spawner = this;
		SpawnedActor->Init();

		SpawnedEnemies.Add(SpawnedActor);
		
	}
}

// Called every frame
void UEnemySpawnerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


