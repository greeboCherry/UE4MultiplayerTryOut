// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"


UCLASS()
class DISKHUNT_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();

	UPROPERTY(VisibleAnywhere, Category = Mesh)
	class UStaticMeshComponent* Represetnation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// To be called after ServerOnUse for items that won't be destroyed
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void PutOnCooldown();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float timeBetweenUses = 20.0f; /*Error: Const properties are not supported. Pfffff */
		
	UPROPERTY(Replicated)
	float TimeUntilNextUse = 0.0f;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool bCanBeUsed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FString Description = "Default Description";

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	FString GetDescription() const;

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	bool CanBeUsed() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnUse(AActor* user);
};
