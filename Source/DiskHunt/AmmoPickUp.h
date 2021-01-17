// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "AmmoPickUp.generated.h"

/**
 * 
 */
UCLASS()
class DISKHUNT_API AAmmoPickUp : public AInteractable
{
	GENERATED_BODY()

public:
	AAmmoPickUp();
	int32 AmmoSize = 10;
	virtual void OnUse_Implementation(AActor* user) override;
};
