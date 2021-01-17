// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "ColorChanger.generated.h"

/**
 * 
 */
UCLASS()
class DISKHUNT_API AColorChanger : public AInteractable
{
	GENERATED_BODY()

	AColorChanger();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UMaterialInstance* ColorMaterial;

	virtual void OnUse_Implementation(AActor* user) override;
};
