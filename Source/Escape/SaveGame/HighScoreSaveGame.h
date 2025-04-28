#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HighScoreSaveGame.generated.h"

/**
 * UHighScoreSaveGame
 * Stores the high score persistently.
 */
UCLASS()
class ESCAPE_API UHighScoreSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // The highest score saved so far.
    UPROPERTY(VisibleAnywhere, Category = "Score")
    float HighScore;

    UHighScoreSaveGame();
};
