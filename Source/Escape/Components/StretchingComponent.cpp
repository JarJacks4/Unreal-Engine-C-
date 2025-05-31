#include "StretchingComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Components/AudioComponent.h"
#include "../Widgets/ActivityUIWidget.h"
#include "../Widgets/ScoreWidget.h"
#include "../Components/SecondCounterComponent.h"

/**
 *  Constructor for UStretchingComponent.
 * Initializes default values for the component.
 */
UStretchingComponent::UStretchingComponent()
{
    // Disable ticking since we use a timer instead
    PrimaryComponentTick.bCanEverTick = false;
    StretchingMusicComponent = nullptr; // Initialized as null
    StretchingTimer = 0.0f; // Initialize timer counter
    CachedEscapeCharacter = nullptr; // Initialize cached escape character
}

/**
 *  Called when the component is initialized in the game world.
 * Caches a reference to the owning character.
 */
void UStretchingComponent::BeginPlay()
{
    Super::BeginPlay();
    // Cache the owning character for efficiency
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
}

/**
 *  Starts the stretching process if the current state is Idle.
 * If `StretchingDuration` > 0, sets a timer; if 0, acts as a toggle.
 */
void UStretchingComponent::StartStretching()
{
    // Ensure OwningCharacter is valid before proceeding
    if (!CachedEscapeCharacter) return;

    // Clear any existing timers before starting stretching
    ClearStretchingTimer();

    // Only start if currently idleg
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Idle) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Starting Stretching..."));
        HandleStretchingStart();
    }
}

/**
 *  Stops the stretching process if the current state is Stretching.
 * Clears the timer if applicable and resets to Idle.
 */
void UStretchingComponent::StopStretching()
{
    if (!CachedEscapeCharacter) return;

    // Only stop if currently stretching
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Stretching)
    {
        HandleStretchingStop();

        // Reset character state and stop the score counter
        if (CachedEscapeCharacter->SecondCounterComponent)
        {
            CachedEscapeCharacter->SecondCounterComponent->StopCounter();
        }

        // Reset character state to Idle
        CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);

		if (CachedEscapeCharacter->GetActivityUIWidget() && CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget())
		{
			// Hide the stretching UI elements
			CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget()->SetVisibility(ESlateVisibility::Hidden);
			CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget()->StopRhythmGame();
		}

        // Clear any active timers
        ClearStretchingTimer();
        
        
    }
}

/**
 *  Handles the start of stretching: sets state, plays music, and optionally starts a timer.
 * Also manages UI visibility for the stretching activity.
 */
void UStretchingComponent::HandleStretchingStart()
{
    int32 CurrentCompletionPoints = CompletionPoints_Stretching;
    int32 CurrentMinimumPoints = MinimumPoints_Stretching;

    // Ensure OwningCharacter is valid before proceeding
    if (!CachedEscapeCharacter) return;

    // Set character state and start the score counter
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Stretching);
    
    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CompletionPoints_Stretching;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = StretchingDuration;
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();
        // Immediately update UI
        if (UTimerWidget* TimerWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
        {
            TimerWidget->UpdateTimer(0.0f, TEXT("Time"));
        }
    }
    
    StretchingTimer = 0.0f; // Reset internal timer tracking
    if (CachedEscapeCharacter->GetActivityUIWidget() && CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget())
    {

        CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget()->SetVisibility(ESlateVisibility::Visible);
		CachedEscapeCharacter->GetActivityUIWidget()->GetRythmWidget()->StartRhythmGame();
    }
    // Play ambient music if assigned
    if (StretchingMusic && GetWorld())
    {
        // Stop any existing music before starting new
        if (StretchingMusicComponent && StretchingMusicComponent->IsPlaying())
        {
            StretchingMusicComponent->Stop();
        }
        
        // Create and play the music via an audio component
        StretchingMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), StretchingMusic);
        if (StretchingMusicComponent)
        {
            StretchingMusicComponent->Play();
        }
    }

    // Set timer only if duration is positive
    if (StretchingDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            StretchingTimerHandle,
            this,
            &UStretchingComponent::OnStretchingComplete,
            StretchingDuration,
            false // Single-shot timer
        );
    }
}

/**
 *  Handles the end of stretching: stops music and resets state.
 */
void UStretchingComponent::HandleStretchingStop()
{
    // Stop stretching music if playing
    if (StretchingMusicComponent && StretchingMusicComponent->IsPlaying())
    {
        StretchingMusicComponent->Stop();
    }
}

/**
 *  Called when the timer completes (if `StretchingDuration` > 0).
 * Applies effects and stops stretching.
 */
void UStretchingComponent::OnStretchingComplete()
{
    // Ensure OwningCharacter is valid before proceeding
    if (!CachedEscapeCharacter) return;

    // Only proceed if still stretching (guards against race conditions or manual stops)
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Stretching)
    {
		StopStretching(); // Stop stretching and reset state
        ApplyStretchingEffects(); // Apply any gameplay effects for completing stretching
        // StopStretching(); // Removed: now handled by the counter
    }
}

/**
 *  Applies effects when stretching completes naturally (timer-based only).
 * Example: add flexibility effect or energy boost
 */
void UStretchingComponent::ApplyStretchingEffects()
{
    if (CachedEscapeCharacter)
    {
        // Example: Apply gameplay effects specific to completing stretching
        // Could include stamina boost, flexibility buff, etc.
    }
}

/**
 *  Sets the stretching duration in seconds.
 *  @param Duration The new stretching duration in seconds (0 for toggle mode)
 */
void UStretchingComponent::SetStretchingDuration(float Duration)
{
    // Ensure duration is non-negative
    StretchingDuration = FMath::Max(Duration, 0.0f);
    
    // If stretching is already in progress and timer is active, update the timer
    if (GetWorld() && CachedEscapeCharacter)
    {
        if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Stretching)
        {
            // Clear existing timer
            GetWorld()->GetTimerManager().ClearTimer(StretchingTimerHandle);
            
            // If new duration is > 0, set a new timer with remaining time
            if (StretchingDuration > 0.0f)
            {
                // Calculate remaining time based on new duration and already elapsed time
                float RemainingTime = FMath::Max(StretchingDuration - StretchingTimer, 0.1f);
                
                GetWorld()->GetTimerManager().SetTimer(
                    StretchingTimerHandle,
                    this,
                    &UStretchingComponent::OnStretchingComplete,
                    RemainingTime,
                    false
                );
            }
        }
    }
}

/**
 *  Clears the stretching timer, resetting elapsed time to zero.
 *  Useful when stopping stretching prematurely.
 */
void UStretchingComponent::ClearStretchingTimer()
{
    // Reset the internal timer counter
    StretchingTimer = 0.0f;
    
    // Clear the timer if it's active
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(StretchingTimerHandle);
    }
}

/**
 *  Checks if the stretching timer is currently active.
 *  @return True if stretching timer is running
 */
bool UStretchingComponent::IsStretchingTimerActive() const
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(StretchingTimerHandle);
}

/**
 *  Sets the current stretch state/pose based on user input.
 *  @param NewStretchState The stretching state to transition to
 */
void UStretchingComponent::SetStretchState(EStretchState NewStretchState)
{
    // Only process stretch states if the character is currently in stretching mode
    if (!CachedEscapeCharacter) return;

    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Stretching)
    {
        // Update the current stretch state
        CurrentStretchState = NewStretchState;

        // Apply gameplay effects based on the new stretch state
        switch (CurrentStretchState)
        {
        case EStretchState::StretchLeft:
            // Handle left stretch - could award points, play sound, etc.
            if (CachedEscapeCharacter->SecondCounterComponent)
            {
                // Award points for performing a stretch
                CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(2.0f); // Example: +2 seconds for a stretch
            }
            break;
        case EStretchState::StretchRight:
            // Handle right stretch
            if (CachedEscapeCharacter->SecondCounterComponent)
            {
                CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(2.0f);
            }
            break;
        case EStretchState::StretchUp:
            // Handle up stretch
            if (CachedEscapeCharacter->SecondCounterComponent)
            {
                CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(2.0f);
            }
            break;
        case EStretchState::StretchDown:
            // Handle down stretch
            if (CachedEscapeCharacter->SecondCounterComponent)
            {
                CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(2.0f);
            }
            break;
        default:
            // Reset to default pose if needed
            break;
        }
    }
    else
    {
        // If not in stretching mode, can't set a stretch state
    }
}