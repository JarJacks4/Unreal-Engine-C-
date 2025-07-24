#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h" // Changed from GameInstanceSubsystem as it's an ActorComponent
#include "Sound/SoundBase.h" // Include for USoundBase
#include "WellnessComponent.generated.h"

// Forward declarations
class UMessageWidget;
class UWellnessSaveGame;
class UTextBlock; // Forward declare for MessageWidget interaction

/**
 *  Structure to hold data for a single wellness message (Affirmation, Gratitude, Hydration).
 */
USTRUCT(BlueprintType)
struct ESCAPE_API FMessage
{
    GENERATED_USTRUCT_BODY()

public:
    /**  The text content of the message. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    FText Text;

    /**  Optional voice clip associated with the message (e.g., for affirmations). Assign in Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message")
    TObjectPtr<USoundBase> VoiceClip; // Use TObjectPtr
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
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent)) // Standard UCLASS definition for ActorComponent
class ESCAPE_API UWellnessComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**  Default constructor. */
    UWellnessComponent(); // Added constructor declaration

    // --- Configuration ---
    /**  Array of affirmation messages. Populate this array in the Blueprint Editor with text and optional voice clips. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content")
    TArray<FMessage> Affirmations;

    /**  The text to display for the gratitude prompt. Set in the Blueprint Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content", meta = (MultiLine = true)) // Added MultiLine
    FText GratitudeFT = FText::FromString(TEXT("Take a moment to think about something you are grateful for today.")); // Improved default text

    /**  The text to display for the hydration reminder. Set in the Blueprint Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Content", meta = (MultiLine = true)) // Added MultiLine
    FText HydrationReminderFT = FText::FromString(TEXT("Remember to stay hydrated! Drink some water.")); // Improved default text

    /**
     *  The UMessageWidget class to instantiate for displaying the wellness messages.
     * Assign the corresponding Blueprint asset (e.g., WBP_MessageWidget) in the Editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wellness Message|Setup") // Changed to BlueprintReadOnly as it's set once
    TSubclassOf<UMessageWidget> MessageWidgetClass;

    /**  If true, plays the associated VoiceClip for affirmations when shown. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Options")
    bool bEnableAffirmationVoice = false;

    /**  Interval (in seconds) between hydration reminders. Default is 5 minutes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "10.0", UIMin = "10.0")) // Added ClampMin
    float HydrationInterval = 300.0f; // 5 minutes

    /**  Interval (in seconds) between gratitude prompts. Default is 4 hours. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "60.0", UIMin = "60.0")) // Added ClampMin
    float GratitudeInterval = 14400.0f; // 4 hours

    /**  Interval (in seconds) between affirmation messages. Default is 5 minutes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "10.0", UIMin = "10.0")) // Added ClampMin
    float AffirmationInterval = 300.0f; // 5 minutes

    /**  Duration (in seconds) for which each message prompt stays visible on screen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness Message|Timing", meta = (ClampMin = "1.0", UIMin = "1.0")) // Added ClampMin
    float MessageDisplayDuration = 10.0f;

    /**  The name of the save game slot used to store wellness data (e.g., last gratitude time). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    FString WellnessSaveSlotName = TEXT("WellnessRemindersSave");

    /**  The user index associated with the wellness save game slot. Typically 0 for single-player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    int32 WellnessUserIndex = 0;



    /** Interval between wellness prompts in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness", meta = (ClampMin = "1.0", UIMin = "1.0"))
    float WellnessPromptInterval = 600.0f;

    // --- Public Methods ---

    /**
     *  Displays the hydration reminder text in the MessageWidget and schedules its removal.
     * Called periodically by HydrationTimerHandle.
     */
    UFUNCTION() // Mark as UFUNCTION if called by timer
    void ShowHydrationReminder();



    /**
     *  Selects and displays a random affirmation from the Affirmations array in the MessageWidget.
     * Plays the voice clip if enabled. Schedules the message removal.
     * Called periodically by AffirmationTimerHandle.
     */
    UFUNCTION(BlueprintCallable) // Mark as UFUNCTION if called by timer
    void ShowAffirmationReminder();

    /**
     *  Displays the gratitude prompt text in the MessageWidget.
     * Updates the LastGratitudePromptTime in the save game. Schedules the message removal.
     * Called periodically by GratitudeTimerHandle, respecting the cooldown from the save game.
     */
    UFUNCTION(BlueprintCallable) // Mark as UFUNCTION if called by timer
    void ShowGratitudePrompt();


protected:
    /**
     *  Called when the component is initialized.
     * Creates the MessageWidget instance, adds it to the viewport, loads the wellness save game,
     * and sets up the repeating timers for hydration, affirmations, and gratitude prompts.
     */
    virtual void BeginPlay() override;

    /**  Called when the component is destroyed or the game ends. Clears active timers. */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // Added EndPlay

private:
    /**  Pointer to the instantiated UMessageWidget used to display prompts. */
    UPROPERTY(Transient) // Transient, created at runtime
    TObjectPtr<UMessageWidget> MessageWidget;

    // --- Timer Handles ---
    /**  Timer handle for the repeating hydration reminder calls. */
    FTimerHandle HydrationTimerHandle;
    /**  Timer handle for the repeating gratitude prompt calls. */
    FTimerHandle GratitudeTimerHandle;
    /**  Timer handle for the repeating affirmation calls. */
    FTimerHandle AffirmationTimerHandle;

    /**  Timer handle used to schedule the removal (hiding) of the currently displayed hydration message. */
    FTimerHandle HydrationRemovalTimerHandle;
    /**  Timer handle used to schedule the removal (hiding) of the currently displayed gratitude message. */
    FTimerHandle GratitudeRemovalTimerHandle;
    /**  Timer handle used to schedule the removal (hiding) of the currently displayed affirmation message. */
    FTimerHandle AffirmationRemovalTimerHandle;

    // --- Save Game ---
    /**  Pointer to the loaded or created UWellnessSaveGame instance. */
    UPROPERTY(Transient) // Transient, loaded/created at runtime
    TObjectPtr<UWellnessSaveGame> SaveGameInstance;

    /**  Timestamp of when the gratitude prompt was last shown, loaded from SaveGameInstance. Used for cooldown logic. */
    FDateTime LastGratitudePromptTime;

    // --- Helper Functions ---
    /**  Loads the UWellnessSaveGame object from the specified slot or creates a new one if it doesn't exist. */
    void LoadOrCreateSaveGame();

    /**  Saves the current state (specifically LastGratitudePromptTime) to the UWellnessSaveGame object. */
    void SaveWellnessData();

    /**  Clears all active timers managed by this component. Called in EndPlay. */
    void ClearAllTimers();

    /**
     *  Helper function to display a message in a specific TextBlock of the MessageWidget and schedule its removal.
     *  TargetTextBlock The TextBlock widget to display the message in (e.g., MessageWidget->GetAffirmationT()).
     *  TextToShow The FText content of the message.
     *  RemovalTimerHandle A reference to the FTimerHandle used to track the removal timer for this specific message type.
     *  OptionalSound Optional USoundBase to play when the message appears.
     */
    void DisplayMessageAndScheduleRemoval(UTextBlock* TargetTextBlock, const FText& TextToShow, FTimerHandle& RemovalTimerHandle, USoundBase* OptionalSound = nullptr);
};
