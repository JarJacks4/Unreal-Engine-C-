// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WellnessSaveGame.generated.h"

/**
 *  UWellnessSaveGame
 * A SaveGame object specifically designed to store data related to the player's wellness activities and progress.
 * Currently, it only stores the timestamp of the last shown gratitude prompt, which can be used
 * to manage cooldowns or track frequency for wellness features.
 *
 * This class can be expanded to save other wellness-related data like meditation streaks,
 * hydration logs, affirmation preferences, etc.
 */
UCLASS()
class ESCAPE_API UWellnessSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /**
     *  Stores the date and time when the gratitude prompt was last successfully displayed to the player.
     * This property is marked with UPROPERTY() to ensure it's automatically handled by Unreal's serialization
     * system when saving and loading the game state.
     * 'VisibleAnywhere' allows viewing this data in the editor/debugger without allowing modification outside the game logic.
     */
    UPROPERTY(VisibleAnywhere, Category = "Wellness Data")
    FDateTime LastGratitudePromptTime;

    UPROPERTY(VisibleAnywhere, Category = "Wellness Data")
    float AggregatedScore = 0.0f;
};
