// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/BaseWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Enemies/EnemyBase.h"


// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("PlayerWeapon"));

	CollisionPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint1"));
	CollisionPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint2"));
	CollisionPoint3 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint3"));
	CollisionPoint4 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint4"));
	CollisionPoint5 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint5"));
	CollisionPoint6 = CreateDefaultSubobject<USceneComponent>(TEXT("CollisionPoint6"));

	CollisionPoint1->SetupAttachment(Mesh);
	CollisionPoint2->SetupAttachment(Mesh);
	CollisionPoint3->SetupAttachment(Mesh);
	CollisionPoint4->SetupAttachment(Mesh);
	CollisionPoint5->SetupAttachment(Mesh);
	CollisionPoint6->SetupAttachment(Mesh);

	CollisionPoints.Add(CollisionPoint1);
	CollisionPoints.Add(CollisionPoint2);
	CollisionPoints.Add(CollisionPoint3);
	CollisionPoints.Add(CollisionPoint4);
	CollisionPoints.Add(CollisionPoint5);
	CollisionPoints.Add(CollisionPoint6);

}

void ABaseWeapon::Activate()
{
}

void ABaseWeapon::Deactvate()
{
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	CollisionPoints.Add(CollisionPoint1);

}

float ABaseWeapon::GetClosestCollisionDistance(AActor* Actor)
{
	if (!Actor) return -1.f;
	AEnemyBase* Enemy = (AEnemyBase*)Actor;
	if (!Enemy) return -1.f;

	float ClosestDistance = MAX_FLT;

	for (USceneComponent* Comp : CollisionPoints)
	{
		FName ClosestBoneName = Enemy->SkelMesh->FindClosestBone(Comp->GetComponentLocation());
		FVector Loc = Enemy->SkelMesh->GetBoneLocation(ClosestBoneName, EBoneSpaces::WorldSpace);

		float TempDist = FVector::Dist(Loc, Comp->GetComponentLocation());

		if (ClosestDistance > TempDist)
		{
			ClosestDistance = TempDist;
		}
	}

	return ClosestDistance;
	
}
