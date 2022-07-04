// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/SlashIndicator.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ASlashIndicator::ASlashIndicator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	GoalIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("Goal"));
	GoalIndicator->SetupAttachment(Root);
	MovingIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("Moving"));
	MovingIndicator->SetupAttachment(Root);
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

