// Fill out your copyright notice in the Description page of Project Settings.


#include "ColorChanger.h"
#include <DiskHunt/DiskHuntCharacter.h>

AColorChanger::AColorChanger()
{
    Description = "Change your color";
    timeBetweenUses = 5.0f;
}

void AColorChanger::OnUse_Implementation(AActor* user)
{
    ADiskHuntCharacter* character = Cast<ADiskHuntCharacter>(user);
    if (!character)
    {
        UE_LOG(LogTemp, Warning, TEXT("AColorChanger::OnUse_Implementation: character not valid"));
        return;
    }
    character->SetMaterial(ColorMaterial);
    PutOnCooldown();
}