#include "WellnessComponent.h"
#include "../Widgets/MessageWidget.h" // Include MessageWidget header
#include "Components/TextBlock.h" // Include TextBlock header
#include "TimerManager.h" // Include for GetWorldTimerManager
#include "Kismet/GameplayStatics.h" // Include for SaveGame functions and PlaySound2D
#include "Blueprint/UserWidget.h" // Include for CreateWidget
#include "../WellnessSaveGame.h" // Include SaveGame class header
#include "Sound/SoundBase.h" // Include for USoundBase
#include "../EscapeCharacter.h" // For debug flag access

/**
 *  Constructor for UWellnessComponent.
 * Initializes default values. Ticking is usually not required as timers handle scheduling.
 */
UWellnessComponent::UWellnessComponent()
{
    // Disable ticking if not needed for continuous updates (timers are used instead).
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize pointers to null.
    MessageWidget = nullptr;
    SaveGameInstance = nullptr;
    LastGratitudePromptTime = FDateTime::MinValue(); // Initialize to a very old date
}

/**
 *  Called when the component is initialized (BeginPlay on the owning Actor).
 * Creates the MessageWidget, loads save data, and starts the timers for wellness prompts.
 */
void UWellnessComponent::BeginPlay()
{
    Super::BeginPlay();

    // --- Widget Creation ---
    // Ensure MessageWidgetClass is assigned in the editor.
    if (!MessageWidgetClass)
    {
        return; // Cannot proceed without the widget class
    }

    // Create the MessageWidget instance.
    MessageWidget = CreateWidget<UMessageWidget>(GetWorld(), MessageWidgetClass);
    if (!MessageWidget)
    {
        return; // Cannot proceed without the widget instance
    }

    // Add the widget to the viewport.
    MessageWidget->AddToViewport();
    // Ensure all text blocks within the message widget are initially hidden.
    if (MessageWidget->GetHydrationReminderT()) MessageWidget->GetHydrationReminderT()->SetVisibility(ESlateVisibility::Collapsed);
    if (MessageWidget->GetGratitudeT()) MessageWidget->GetGratitudeT()->SetVisibility(ESlateVisibility::Collapsed);
    if (MessageWidget->GetAffirmationT()) MessageWidget->GetAffirmationT()->SetVisibility(ESlateVisibility::Collapsed);


    // --- Load Save Game ---
    LoadOrCreateSaveGame();

    // --- Start Timers ---
    // Ensure intervals are valid (positive) before setting timers.
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (AffirmationInterval > 0.0f)
    {
        // Set a repeating timer for affirmations. The first call happens after AffirmationInterval seconds.
        TimerManager.SetTimer(AffirmationTimerHandle, this, &UWellnessComponent::ShowAffirmationReminder, AffirmationInterval, true);
    }

    if (HydrationInterval > 0.0f)
    {
        // Set a repeating timer for hydration reminders.
        TimerManager.SetTimer(HydrationTimerHandle, this, &UWellnessComponent::ShowHydrationReminder, HydrationInterval, true);
    }

    if (GratitudeInterval > 0.0f)
    {
        // Set a repeating timer for gratitude prompts.
        // The ShowGratitudePrompt function itself will check the cooldown based on LastGratitudePromptTime.
        TimerManager.SetTimer(GratitudeTimerHandle, this, &UWellnessComponent::ShowGratitudePrompt, GratitudeInterval, true);
    }
}

/**
 *  Called when the component is being destroyed or the game is ending.
 * Clears all active timers managed by this component to prevent issues.
 *  EndPlayReason The reason why EndPlay is being called.
 */
void UWellnessComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllTimers(); // Ensure timers are cleared on destruction
    Super::EndPlay(EndPlayReason);
}

/**
 *  Displays the gratitude prompt if enough time has passed since the last prompt.
 * Updates the last prompt time in the save game if shown.
 */
void UWellnessComponent::ShowGratitudePrompt()
{
    // Check if the MessageWidget is valid.
    if (!MessageWidget || !MessageWidget->GetGratitudeT())
    {
        return;
    }

    // Check cooldown: Ensure enough time has passed since the last prompt.
    const FDateTime CurrentTime = FDateTime::UtcNow(); // Use UTC for consistency
    const FTimespan TimeSinceLastPrompt = CurrentTime - LastGratitudePromptTime;
    const FTimespan RequiredInterval = FTimespan::FromSeconds(GratitudeInterval);

    if (TimeSinceLastPrompt >= RequiredInterval)
    {
        // Enough time has passed, show the prompt.
        DisplayMessageAndScheduleRemoval(
            MessageWidget->GetGratitudeT(),
            GratitudeFT,
            GratitudeRemovalTimerHandle
        );

        // Update the last shown time and save it.
        LastGratitudePromptTime = CurrentTime;
        SaveWellnessData();
    }
    else
    {
        // Not enough time has passed, skip showing the prompt this time.
    }
}

/**
 *  Displays the hydration reminder message.
 */
void UWellnessComponent::ShowHydrationReminder()
{
    // Check if the MessageWidget is valid.
    if (!MessageWidget || !MessageWidget->GetHydrationReminderT())
    {
        return;
    }

    DisplayMessageAndScheduleRemoval(
        MessageWidget->GetHydrationReminderT(),
        HydrationReminderFT,
        HydrationRemovalTimerHandle
    );
}

/**
 *  Selects and displays a random affirmation message. Plays audio if enabled.
 */
void UWellnessComponent::ShowAffirmationReminder()
{
    // Check if the MessageWidget is valid and if there are any affirmations configured.
    if (!MessageWidget || !MessageWidget->GetAffirmationT())
    {
        return;
    }
    if (Affirmations.Num() == 0)
    {
        return; // No affirmations to show
    }

    // Select a random affirmation from the array.
    const int32 RandomIndex = FMath::RandRange(0, Affirmations.Num() - 1);
    const FMessage& RandomAffirmation = Affirmations[RandomIndex];

    // Determine if sound should be played.
    USoundBase* SoundToPlay = (bEnableAffirmationVoice && RandomAffirmation.VoiceClip) ? RandomAffirmation.VoiceClip : nullptr;

    // Display the affirmation text and potentially play the sound.
    DisplayMessageAndScheduleRemoval(
        MessageWidget->GetAffirmationT(),
        RandomAffirmation.Text,
        AffirmationRemovalTimerHandle,
        SoundToPlay
    );
}

/**
 *  Helper function to display text in a specific TextBlock, play an optional sound, and schedule the TextBlock to be hidden later.
 *  TargetTextBlock The TextBlock to modify.
 *  TextToShow The text to display.
 *  RemovalTimerHandle Reference to the timer handle for managing the removal timer.
 *  OptionalSound Sound to play when the message appears.
 */
void UWellnessComponent::DisplayMessageAndScheduleRemoval(UTextBlock* TargetTextBlock, const FText& TextToShow, FTimerHandle& RemovalTimerHandle, USoundBase* OptionalSound)
{
    if (!TargetTextBlock) return; // Safety check

    // Make the text block visible and set its text.
    TargetTextBlock->SetVisibility(ESlateVisibility::Visible);
    TargetTextBlock->SetText(TextToShow);

    // Play sound if provided.
    if (OptionalSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), OptionalSound);
    }

    // Clear any existing removal timer for this specific message type.
    GetWorld()->GetTimerManager().ClearTimer(RemovalTimerHandle);

    // Schedule the widget to be hidden after MessageDisplayDuration seconds.
    // Using a lambda to capture the TargetTextBlock.
    GetWorld()->GetTimerManager().SetTimer(RemovalTimerHandle, [TargetTextBlock]() { // CORRECTED SYNTAX []()
        if (TargetTextBlock)
        {
            TargetTextBlock->SetVisibility(ESlateVisibility::Collapsed);
        }
        }, MessageDisplayDuration, false);

}


/**
 *  Loads the UWellnessSaveGame from the specified slot or creates a new one if none exists.
 * Populates the LastGratitudePromptTime from the loaded data.
 */
void UWellnessComponent::LoadOrCreateSaveGame()
{
    // Attempt to load the save game from the specified slot.
    SaveGameInstance = Cast<UWellnessSaveGame>(UGameplayStatics::LoadGameFromSlot(WellnessSaveSlotName, WellnessUserIndex));

    if (SaveGameInstance)
    {
        // Save game loaded successfully. Retrieve the last prompt time.
        LastGratitudePromptTime = SaveGameInstance->LastGratitudePromptTime;
    }
    else
    {
        // No save game found, create a new one.
        SaveGameInstance = Cast<UWellnessSaveGame>(UGameplayStatics::CreateSaveGameObject(UWellnessSaveGame::StaticClass()));
        if (SaveGameInstance)
        {
            // Initialize default values in the new save game instance.
            LastGratitudePromptTime = FDateTime::MinValue();
            SaveGameInstance->LastGratitudePromptTime = LastGratitudePromptTime;
            // Only save the new object if it didn't exist before
            UGameplayStatics::SaveGameToSlot(SaveGameInstance, WellnessSaveSlotName, WellnessUserIndex);
        }
    }
}

/**
 *  Saves the current LastGratitudePromptTime to the UWellnessSaveGame object in the specified slot.
 */
void UWellnessComponent::SaveWellnessData()
{
    // Always load the save game object first to avoid overwriting other fields (like AggregatedScore)
    UWellnessSaveGame* SaveGameInstanceLocal = Cast<UWellnessSaveGame>(UGameplayStatics::LoadGameFromSlot(WellnessSaveSlotName, WellnessUserIndex));
    if (!SaveGameInstanceLocal)
    {
        SaveGameInstanceLocal = Cast<UWellnessSaveGame>(UGameplayStatics::CreateSaveGameObject(UWellnessSaveGame::StaticClass()));
    }
    if (SaveGameInstanceLocal)
    {
        // Only update the LastGratitudePromptTime, leave other fields (like AggregatedScore) untouched
        SaveGameInstanceLocal->LastGratitudePromptTime = LastGratitudePromptTime;
        UGameplayStatics::SaveGameToSlot(SaveGameInstanceLocal, WellnessSaveSlotName, WellnessUserIndex);
    }
    else
    {
        // Handle the error - unable to create or load the save game instance
    }
}

/**
 *  Clears all timers managed by this component.
 */
void UWellnessComponent::ClearAllTimers()
{
    if (GetWorld()) // Ensure world context is valid
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        TimerManager.ClearTimer(HydrationTimerHandle);
        TimerManager.ClearTimer(GratitudeTimerHandle);
        TimerManager.ClearTimer(AffirmationTimerHandle);
        TimerManager.ClearTimer(HydrationRemovalTimerHandle);
        TimerManager.ClearTimer(GratitudeRemovalTimerHandle);
        TimerManager.ClearTimer(AffirmationRemovalTimerHandle);
    }
}
