// Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "DiskHuntCharacter.h"
#include "DiskHuntGameMode.h"
#include <Runtime/Engine/Public/Net/UnrealNetwork.h>

// Sets default values
ATarget::ATarget()
{
	bReplicates = true;
	bReplicateMovement = true;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetIsReplicated(true);

}

// Called when the game starts or when spawned
void ATarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATarget::RegisterHit(ADiskHuntCharacter* character)
{
	UE_LOG(LogTemp, Log, TEXT("Target was hit!"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Target was hit!");
	if (!bCanBeScored) return;

	Mesh->SetSimulatePhysics(true);
	Mesh->SetMaterial(0, character->PlayerMaterial);
	bCanBeScored = false;
	SetLifeSpan(3.0f);
	//PrimaryActorTick.bCanEverTick = true;
	if (HasAuthority())	ServerScoringLogic(character);
}

void ATarget::ServerScoringLogic_Implementation(ADiskHuntCharacter* character)
{
	SetLifeSpan(2.0f);
	if (bCanBeScored)

	if (ADiskHuntGameMode* gameMode = Cast<ADiskHuntGameMode>(GetWorld()->GetAuthGameMode()))
	{
		UE_LOG(LogTemp, Log, TEXT("AwardScore requested"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "AwardScore requested");
		gameMode->AwardScore(character, 1);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No game mode?"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No game mode?");
	}

}

void ATarget::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATarget, bCanBeScored);
}
