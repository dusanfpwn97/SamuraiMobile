// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemies/EnemySpawnerComponent.h"
#include "Misc/Pool/PoolManager.h"


// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	RootComponent = Capsule;

	SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkelMesh->SetupAttachment(Capsule);
}

void AEnemyBase::Die()
{
	UWorld* World = GetWorld();
	if (!World) return;

	IsAlive = false;

	World->GetTimerManager().SetTimer(ReleaseToPoolTH, this, &AEnemyBase::ReleaseToPool, 5.f, false);

	ActivateRagdoll();
}

void AEnemyBase::Init()
{
	IsAlive = true;
}

void AEnemyBase::ReleaseToPool()
{
	if (Spawner)
	{
		Spawner->ReleaseToPool(this);
	}
	else
	{
		Destroy();
	}
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemyBase::ActivateRagdoll()
{
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);

	SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));

	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetSimulatePhysics(true);
	SkelMesh->WakeAllRigidBodies();
	SkelMesh->bBlendPhysics = true;
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

