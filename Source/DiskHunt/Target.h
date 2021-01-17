// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiskHuntCharacter.h"
#include "Target.generated.h"

UCLASS()
class DISKHUNT_API ATarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATarget();

	/** Target's Body */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* Mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Replicated, Category = Gameplay)
	bool bCanBeScored = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void RegisterHit(ADiskHuntCharacter* character);

	UFUNCTION(Server, reliable)
	void ServerScoringLogic(ADiskHuntCharacter* character);
};
