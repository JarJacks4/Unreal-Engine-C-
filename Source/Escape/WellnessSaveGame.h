// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WellnessSaveGame.generated.h"

/**
 * Save game class for storing wellness-related data, specifically the last gratitude prompt time.
 */
UCLASS()
class ESCAPE_API UWellnessSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
    // Last time the gratitude prompt was displayed
    // Exposed to the Editor and serialization system
    UPROPERTY()
    FDateTime LastGratitudePromptTime;
};
