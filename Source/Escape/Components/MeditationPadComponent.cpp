#include "MeditationPadComponent.h" 
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Components/AudioComponent.h"
#include "../Widgets/ActivityUIWidget.h" // Include Activity UI Widget header
#include "../Widgets/BreathingBubbleWidget.h" // Include Breathing Bubble Widget header
#include "../Widgets/ScoreWidget.h" // Include Score Widget header
#include "../Widgets/TimerWidget.h" // Include Timer Widget header
#include "Components/Image.h" // Include Image header for TargetBubble
#include "../Components/SecondCounterComponent.h"

/**
 *  Constructor for UMeditationPadComponent.
 * Initializes default values for the component.
 */
UMeditationPadComponent::UMeditationPadComponent() // Changed constructor name
{
    // Disable ticking since we use a timer instead
    PrimaryComponentTick.bCanEverTick = false;
    MeditationMusic = nullptr; // Changed variable name
    CachedEscapeCharacter = nullptr; // Initialize cached escape character
}

/**
 *  Called when the component is initialized in the game world.
 * Caches a reference to the owning character.
 */
void UMeditationPadComponent::BeginPlay() // Changed class scope
{
    Super::BeginPlay();
    // Cache the owning character for efficiency
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner()); // Cache the cast
}

/**
 *  Starts the meditation pad interaction if the current state is Idle.
 * If `InteractionDuration` > 0, sets a timer; if 0, acts as a toggle.
 */
void UMeditationPadComponent::StartMeditation() // Changed function name
{
    // Ensure OwningCharacter is valid before proceeding

    if (!CachedEscapeCharacter) return;

    // Clear any existing timers before starting interaction
    ClearMeditationTimer(); // Changed function call

    // Only start if currently idle
    // Assuming Meditation Pad uses the same state machine for now
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Idle) {
        HandleMeditationStart(); // Changed function call
    }
}

/**
 *  Stops the meditation pad interaction if the current state is Interacting.
 * Clears the timer if applicable and resets to Idle.
 */
void UMeditationPadComponent::StopMeditation() // Changed function name
{

   
    if (!CachedEscapeCharacter) return;

    //Only stop if currently interacting (assuming a new state or reusing Meditating for now)
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating) // Reusing Meditating state for now
    {
        HandleMeditationStop(); // Changed function call

        // Hide the activity UI (or keep score visible)
        if (UActivityUIWidget* ActivityWidget = CachedEscapeCharacter->GetActivityUIWidget())
        {
            // ActivityWidget->SetVisibility(ESlateVisibility::Collapsed); // Optional: Keep score visible
        }

        // Reset block state (assuming Meditation Pad uses WellnessBlock and its states)
        if (BlockRef)
        {
            // TODO: Add specific states for Meditation Pad block if different from Meditation
            BlockRef->SetMeditationBlockState(EMeditationBlockState::Lowering); // Reusing Meditation states for now
        }

        // Stop the score counter
        if (CachedEscapeCharacter->SecondCounterComponent)
        {
            CachedEscapeCharacter->SecondCounterComponent->StopCounter();
        }
    }
}

/**
 *  Handles the start of interaction: sets state, plays music, and optionally starts a timer.
 * Also manages UI visibility for the interaction activity.
 */
void UMeditationPadComponent::HandleMeditationStart() // Changed function name
{

    if (!CachedEscapeCharacter) return;

    // Configure and show the Activity UI
    if (UActivityUIWidget* ActivityWidget = CachedEscapeCharacter->GetActivityUIWidget())
    {
        // Hide elements not used in this interaction (adjust as needed)
        if (UImage* TargetBubble = ActivityWidget->GetTargetBubble())
        {
            TargetBubble->SetVisibility(ESlateVisibility::Hidden);
        }
        if (UBreathingBubbleWidget* BreathingWidget = ActivityWidget->GetDeepBreathingWidget())
        {
            BreathingWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
        // Make the main activity widget visible
        ActivityWidget->SetVisibility(ESlateVisibility::Visible);
    }

    // Set character state and start the score counter
    // TODO: Use a dedicated state if created
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Meditating); // Reusing Meditating state
    // TODO: Use dedicated block states if created
    if (BlockRef) BlockRef->SetMeditationBlockState(EMeditationBlockState::Rising); // Reusing Meditation state
    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CompletionPoints;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = MeditationDuration; // Changed variable
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();
        // Immediately update the UI with the initial values
        if (UTimerWidget* TimerWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
        {
            TimerWidget->UpdateTimer(0.0f, TEXT("Time"));
        }
    }
    MeditationTimer = 0.0f; // Changed variable name

    // Play ambient music if assigned
    if (MeditationMusic && GetWorld()) // Changed variable name
    {
        // Stop any existing music before starting new
        if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying()) // Changed variable name
        {
            MeditationMusicComponent->Stop(); // Changed variable name
        }
        // Create and play the music via an audio component
        MeditationMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), MeditationMusic); // Changed variable names
        if (MeditationMusicComponent) // Changed variable name
        {
            MeditationMusicComponent->Play(); // Changed variable name
        }
    }

    // Set timer only if duration is positive
    if (MeditationDuration > 0.0f && GetWorld()) // Changed variable name
    {
        GetWorld()->GetTimerManager().SetTimer(
            MeditationTimerHandle, // Changed variable name
            this,
            &UMeditationPadComponent::OnMeditationComplete, // Changed function name
            MeditationDuration, // Changed variable name
            false // Single-shot timer
        );
    }
}

/**
 *  Handles the end of interaction: stops music and resets state.
 */
void UMeditationPadComponent::HandleMeditationStop() // Changed function name
{
    // TODO: Use dedicated block states if created
    if (BlockRef) BlockRef->SetMeditationBlockState(EMeditationBlockState::Lowering); // Reset block state (reusing)
    // Stop interaction music if playing
    if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying()) // Changed variable name
    {
        MeditationMusicComponent->Stop(); // Changed variable name
    }
    // Note: State changes are handled in StopInteraction() or OnInteractionComplete()
}

/**
 *  Called when the timer completes (if `InteractionDuration` > 0).
 * Applies effects and stops interaction.
 */
void UMeditationPadComponent::OnMeditationComplete() // Changed function name
{
    if (!CachedEscapeCharacter) return;

    // Only proceed if still interacting (guards against race conditions or manual stops)
    // TODO: Use dedicated state if created
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating) // Reusing state
    {
        ApplyMeditationEffects(); // Changed function call
        StopMeditation(); // Ensure block state is set to Lowering and counter is stopped // Changed function call
    }
}

/**
 *  Applies effects when interaction completes naturally (timer-based only).
 * Placeholder for potential gameplay effects.
 */
void UMeditationPadComponent::ApplyMeditationEffects() // Changed function name
{
    if (CachedEscapeCharacter)
    {
        // Example: Log completion or apply a gameplay effect
        // Add gameplay logic here
    }
}

/**
 *  Sets the interaction duration in seconds.
 *  @param Duration The new interaction duration in seconds (0 for toggle mode)
 */
void UMeditationPadComponent::SetMeditationDuration(float Duration) // Changed function name
{
    // Ensure duration is non-negative
    MeditationDuration = FMath::Max(Duration, 0.0f); // Changed variable name

    // If interaction is already in progress and timer is active, update the timer
    if (GetWorld() && CachedEscapeCharacter)
    {
        // TODO: Use dedicated state if created
        if (CachedEscapeCharacter && CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating) // Reusing state
        {
            // Clear existing timer
            GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle); // Changed variable name

            // If new duration is > 0, set a new timer with remaining time
            if (MeditationDuration > 0.0f) // Changed variable name
            {
                // Calculate remaining time based on new duration and already elapsed time
                float RemainingTime = FMath::Max(MeditationDuration - MeditationTimer, 0.1f); // Changed variable names

                GetWorld()->GetTimerManager().SetTimer(
                    MeditationTimerHandle, // Changed variable name
                    this,
                    &UMeditationPadComponent::OnMeditationComplete, // Changed function name
                    RemainingTime,
                    false
                );
            }
        }
    }
}

/**
 *  Clears the interaction timer, resetting elapsed time to zero.
 *  Useful when stopping interaction prematurely.
 */
void UMeditationPadComponent::ClearMeditationTimer() // Changed function name
{
    // Reset the internal timer counter
    MeditationTimer = 0.0f; // Changed variable name

    // Clear the timer if it's active
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle); // Changed variable name
    }
}

/**
 *  Checks if the interaction timer is currently active.
 *  @return True if interaction timer is running
 */
bool UMeditationPadComponent::IsMeditationTimerActive() const // Changed function name
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(MeditationTimerHandle); // Changed variable name
}
