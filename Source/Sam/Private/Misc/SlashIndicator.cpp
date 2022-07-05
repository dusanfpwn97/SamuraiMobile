// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/SlashIndicator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ASlashIndicator::ASlashIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	Goal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Goal"));
	Goal->SetupAttachment(Root);
	Moving = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Moving"));
	Moving->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void ASlashIndicator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlashIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

