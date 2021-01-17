// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DiskHuntCharacter.h"
#include "DiskHuntGameMode.generated.h"


UCLASS(minimalapi)
class ADiskHuntGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADiskHuntGameMode();

	void AwardScore(ADiskHuntCharacter* character, int32 score);

	TMap<ADiskHuntCharacter*, int32> CharactersScore;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	int32 GetScore(ADiskHuntCharacter* character);
};



