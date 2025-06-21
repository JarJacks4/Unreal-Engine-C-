#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HighScoreSaveGame.generated.h"

/**
 *  UHighScoreSaveGame
 * A simple SaveGame object designed specifically to store a single high score value (as a float).
 * This is used by the USecondCounterComponent to persist the best time achieved for different activities
 * (Meditation, Stretching, Deep Breathing) across game sessions. Each activity uses a separate instance
 * of this save game object, identified by a unique SaveSlotName.
 */
UCLASS()
class ESCAPE_API UHighScoreSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /**
     *  The highest score (typically elapsed time in seconds) recorded for the activity associated with this save slot.
     * Marked with UPROPERTY() to ensure it's saved and loaded correctly by Unreal's serialization system.
     * 'VisibleAnywhere' allows viewing in the editor/debugger. Defaults to 0.0f.
     */
    UPROPERTY(VisibleAnywhere, Category = "High Score")
    float HighScore = 0.0f;
};
