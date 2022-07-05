// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SlashIndicator.generated.h"

UCLASS()
class ASlashIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASlashIndicator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Goal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* Moving;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
