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
	CollisionPoints.Add(CollisionPoint2);
	CollisionPoints.Add(CollisionPoint3);
	CollisionPoints.Add(CollisionPoint4);
	CollisionPoints.Add(CollisionPoint5);
	CollisionPoints.Add(CollisionPoint6);
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShouldCheck)
	{
		CheckCollision();
	}
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

void ABaseWeapon::StartCheckingCollision()
{
	UWorld* World = GetWorld(); if (!World) return;
	CollidedActors.Empty();

	World->GetTimerManager().SetTimer(CheckCollitionTH, this, &ABaseWeapon::CheckCollision, 0.017f, true);
	ShouldCheck = true;
}

void ABaseWeapon::CheckCollision()
{/*
	TArray<UPrimitiveComponent*> Comps;
	Mesh->GetOverlappingComponents(Comps);

	if (Comps.Num() == 0) return;

	for (UPrimitiveComponent* Comp : Comps)
	{
		AActor* TempOwner = Comp->GetOwner();
		if (TempOwner)
		{
			if (TempOwner->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()) && Comp->GetClass() == USkeletalMeshComponent::StaticClass())
			{
				ICombatInterface* TempInterface = Cast<ICombatInterface>(TempOwner);
				if (!TempInterface) return;
				TempInterface->OnWeaponCollided(TempOwner, "None");
				CollidedActors.AddUnique(TempOwner);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("COLLISONSDWDQWDQWDWQ")));
			}
		}
	}
	
	*/
	TArray<AActor*> Actors;
	Mesh->GetOverlappingActors(Actors, AEnemyBase::StaticClass());
	
	if (Actors.Num() == 0) return;

	for (AActor* Act : Actors)
	{
		if(Act->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
		{
			ICombatInterface* TempInterface = Cast<ICombatInterface>(Act);
			if (!TempInterface) return;
			TempInterface->OnWeaponHit(Act, "None");
			CollidedActors.AddUnique(Act);
			
			if (Owner)
			{
				if (Owner->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
				{
					ICombatInterface* TempInterface2 = Cast<ICombatInterface>(Owner);
					if (!TempInterface2) return;
					TempInterface2->OnWeaponHitEnemy(Act, "None");
				}
			}
		}
	}
}

void ABaseWeapon::StopCheckingCollision()
{
	UWorld* World = GetWorld(); if (!World) return;

	World->GetTimerManager().ClearTimer(CheckCollitionTH);
	CollidedActors.Empty();

	ShouldCheck = false;
}
