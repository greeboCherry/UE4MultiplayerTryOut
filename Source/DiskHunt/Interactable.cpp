// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable.h"
#include "Classes/Components/StaticMeshComponent.h"
#include <Runtime/Engine/Public/Net/UnrealNetwork.h>

// Sets default values
AInteractable::AInteractable()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Replication
    bReplicates = true;

    // Set up some mesh with proper collision channel
    Represetnation = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Represetnation"));
    Represetnation -> SetCollisionProfileName("Interactable");
    RootComponent = Represetnation;
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AInteractable::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!CanBeUsed())
    {
        TimeUntilNextUse -= DeltaTime;
        if (TimeUntilNextUse < 0.0f)
        {
            bCanBeUsed = true;
        }
    }

}

FString AInteractable::GetDescription() const
{
    if (CanBeUsed())
    {
        return Description;
    }
    else
    {
        return Description + " - inactive for " + FString::SanitizeFloat(TimeUntilNextUse, 2) + " seconds";
    }
}

bool AInteractable::CanBeUsed() const
{
    return bCanBeUsed;
}

void AInteractable::OnUse_Implementation(AActor* user)
{
    throw std::exception("AInteractable::OnUse NYI");
}

void AInteractable::PutOnCooldown()
{
    bCanBeUsed = false;
    TimeUntilNextUse = timeBetweenUses;
}

void AInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to every client, no special condition required
    DOREPLIFETIME(AInteractable, bCanBeUsed);
    DOREPLIFETIME(AInteractable, TimeUntilNextUse);
}