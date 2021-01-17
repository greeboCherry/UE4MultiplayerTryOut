// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "DiskHuntCharacter.h"

AAmmoPickUp::AAmmoPickUp()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    Description = "Pick up ammo ";
}

void AAmmoPickUp::OnUse_Implementation(AActor* user)
{
    ADiskHuntCharacter* character = Cast<ADiskHuntCharacter>(user);
    character->IncreaseAmmo(AmmoSize);
    Destroy();
}