// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WellnessSaveGame.generated.h"

/**
 *  UWellnessSaveGame
 * A SaveGame object specifically designed to store data related to the player's wellness activities and progress.
 */
UCLASS()
class ESCAPE_API UWellnessSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category = "Wellness Data")
    FDateTime LastGratitudePromptTime;

    UPROPERTY(VisibleAnywhere, Category = "Wellness Data")
    float AggregatedScore = 0.0f;
};
