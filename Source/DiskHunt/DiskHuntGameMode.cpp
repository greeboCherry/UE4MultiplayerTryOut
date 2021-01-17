// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DiskHuntGameMode.h"
#include "DiskHuntHUD.h"
#include "DiskHuntCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerColors.h"

ADiskHuntGameMode::ADiskHuntGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ADiskHuntHUD::StaticClass();
}

void ADiskHuntGameMode::AwardScore(ADiskHuntCharacter* character, int32 score)
{
	UE_LOG(LogTemp, Log, TEXT("AwardScore "));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "AwardScore ");
	const auto it = CharactersScore.Find(character);
	if (it)
	{
		(*it)+=score;
	}
	else
	{
		CharactersScore.Add(character, score);
	}
}

int32 ADiskHuntGameMode::GetScore(ADiskHuntCharacter* character)
{
	const auto it = CharactersScore.Find(character);
	if (it)
	{
		return *it;
	}
	else
	{
		CharactersScore.Add(character, 0);
		return 0;
	}
}
