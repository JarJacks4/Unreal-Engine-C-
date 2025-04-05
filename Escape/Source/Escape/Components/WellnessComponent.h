#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"   
#include "WellnessComponent.generated.h"


USTRUCT(BlueprintType)
struct ESCAPE_API FMessage
{
    GENERATED_USTRUCT_BODY()
public:
    // Text of the affirmation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    FText Text;


    // Optional voice clip for the affirmation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    USoundBase* VoiceClip;
};


class UMessageWidget;
class UWellnessSaveGame;

/**
 * Subsystem responsible for managing periodic wellness prompts, such as hydration reminders and gratitude reflection.
 * Attached to the game instance to ensure it runs throughout the player's session.
 */
UCLASS()
class ESCAPE_API UWellnessComponent : public UActorComponent
{
	GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    TArray<FMessage> Affirmations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    FText GratitudeFT = FText::FromString("Insert the Gratitude prompt here");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    FText HydrationReminderFT = FText::FromString("Insert the Hydration Reminder here");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    TSubclassOf<UMessageWidget> MessageWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    bool bEnableAffirmationVoice = false;

    /**
     * Called when the game starts, setting up timers for hydration, affirmations and gratitude prompts.
     * Loads or creates the save game object for persistence.
     */
    virtual void BeginPlay() override;


    // Interval for hydration reminders in seconds (5 minutes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    float HydrationInterval = 300.0f;

    // Interval for gratitude prompts in seconds (4 hours)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    float GratitudeInterval = 14400.0f;


    // Interval for affirmations in seconds (5 minutes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    float AffirmationInterval = 300.0f;

    /**
     * Displays the hydration reminder Text.
     */
    void ShowHydrationReminder();

    /**
     * Displays the affirmations.
     */
    void ShowAffirmationReminder();

    /**
     * Displays the gratitude promp and updates the last prompt time.
     */
    void ShowGratitudePrompt();

private:

    UMessageWidget* MessageWidget;

    // Timer handle for the hydration UI removal
    FTimerHandle HydrationRemovalTimerHandle;

    // Timer handle for the Gratitude UI removal
    FTimerHandle GratitudeRemovalTimerHandle;

    // Timer handle for the affirmation UI removal
    FTimerHandle AffirmationRemovalTimerHandle;


    // Timer handle for hydration reminders
    FTimerHandle HydrationTimerHandle;

    // Currently displayed hydration widget (if any)
    FTimerHandle GratitudeTimerHandle;


    // Timer handle for hydration reminders
    FTimerHandle AffirmationTimerHandle;


    // Save game object for persisting gratitude prompt timing
    UWellnessSaveGame* SaveGameInstance;

    // Last time the gratitude prompt was shown
    FDateTime LastGratitudePromptTime;

   
	
};
