#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "WellnessComponent.generated.h"

// Forward declarations
class UMessageWidget;
class UWellnessSaveGame;
class UTextBlock;

/**
 *  Structure to hold data for a single wellness message (Affirmation, Gratitude, Hydration).
 */
USTRUCT(BlueprintType)
struct ESCAPE_API FMessage
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    TObjectPtr<USoundBase> VoiceClip;
};

/**
 *  UWellnessComponent
 * An ActorComponent responsible for managing periodic wellness prompts displayed to the player.
 * It handles scheduling and displaying hydration reminders, gratitude prompts, and affirmations
 * using a UMessageWidget. It uses timers to control the frequency and duration of these messages.
 * It also interacts with UWellnessSaveGame to persist the timestamp of the last gratitude prompt shown.
 *
 * This component should be added to the player character Actor (e.g., AEscapeCharacter).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UWellnessComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWellnessComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content")
    TArray<FMessage> Affirmations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content", meta = (MultiLine = true))
    FText GratitudeFT = FText::FromString(TEXT("Take a moment to think about something you are grateful for today."));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content", meta = (MultiLine = true))
    FText HydrationReminderFT = FText::FromString(TEXT("Remember to stay hydrated! Drink some water."));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wellness Message|Setup")
    TSubclassOf<UMessageWidget> MessageWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Options")
    bool bEnableAffirmationVoice = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "10.0", UIMin = "10.0"))
    float HydrationInterval = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "60.0", UIMin = "60.0"))
    float GratitudeInterval = 14400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "10.0", UIMin = "10.0"))
    float AffirmationInterval = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "1.0", UIMin = "1.0"))
    float MessageDisplayDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    FString WellnessSaveSlotName = TEXT("WellnessRemindersSave");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    int32 WellnessUserIndex = 0;

    UFUNCTION()
    void ShowHydrationReminder();

    UFUNCTION(BlueprintCallable)
    void ShowAffirmationReminder();

    UFUNCTION(BlueprintCallable)
    void ShowGratitudePrompt();

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMessageWidget> MessageWidget;

    FTimerHandle HydrationTimerHandle;
    FTimerHandle GratitudeTimerHandle;
    FTimerHandle AffirmationTimerHandle;

    FTimerHandle HydrationRemovalTimerHandle;
    FTimerHandle GratitudeRemovalTimerHandle;
    FTimerHandle AffirmationRemovalTimerHandle;

    UPROPERTY(Transient)
    TObjectPtr<UWellnessSaveGame> SaveGameInstance;

    FDateTime LastGratitudePromptTime;

    void LoadOrCreateSaveGame();

    void SaveWellnessData();

    void ClearAllTimers();

    void DisplayMessageAndScheduleRemoval(UTextBlock* TargetTextBlock, const FText& TextToShow, FTimerHandle& RemovalTimerHandle, USoundBase* OptionalSound = nullptr);
};
