#include "MeditationComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Components/AudioComponent.h"
#include "../Widgets/ActivityUIWidget.h" // Include Activity UI Widget header
#include "../Widgets/BreathingBubbleWidget.h" // Include Breathing Bubble Widget header
#include "../Widgets/ScoreWidget.h" // Include Score Widget header
#include "Components/Image.h" // Include Image header for TargetBubble
#include "../Components/SecondCounterComponent.h"

/**
 *  Constructor for UMeditationComponent.
 * Initializes default values for the component.
 */
UMeditationComponent::UMeditationComponent()
{
    // Disable ticking since we use a timer instead
    PrimaryComponentTick.bCanEverTick = false;
    MeditationMusicComponent = nullptr; // Initialized as null
    CachedEscapeCharacter = nullptr; // Initialize cached escape character
}

/**
 *  Called when the component is initialized in the game world.
 * Caches a reference to the owning character.
 */
void UMeditationComponent::BeginPlay()
{
    Super::BeginPlay();
    // Cache the owning character for efficiency
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner()); // Cache the cast
}

/**
 *  Starts the meditation process if the current state is Idle.
 * If `MeditationDuration` > 0, sets a timer; if 0, acts as a toggle.
 */
void UMeditationComponent::StartMeditation()
{
    // Ensure OwningCharacter is valid before proceeding
    if (!CachedEscapeCharacter) return;

    // Clear any existing timers before starting meditation
    ClearMeditationTimer();

    // Only start if currently idle
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Idle) {
        HandleMeditationStart();
    }
}

/**
 *  Stops the meditation process if the current state is Meditating.
 * Clears the timer if applicable and resets to Idle.
 */
void UMeditationComponent::StopMeditation(){

    if (!CachedEscapeCharacter) return;

    //Only stop if currently meditating
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        HandleMeditationStop();

        

        if (BlockRef)
        {
            BlockRef->SetMeditationBlockState(EMeditationBlockState::Lowering);
        }

        // Stop the score counter
        if (CachedEscapeCharacter->SecondCounterComponent)
        {
            CachedEscapeCharacter->SecondCounterComponent->StopCounter();
        }
    }
}

/**
 *  Handles the start of meditation: sets state, plays music, and optionally starts a timer.
 * Also manages UI visibility and sets points/duration based on MeditationType.
 */
void UMeditationComponent::HandleMeditationStart()
{
    if (!CachedEscapeCharacter) return;

    // Configure and show the Activity UI
    

    // --- Meditation Type Specific Setup ---
    int32 CurrentCompletionPoints = 0;
    int32 CurrentMinimumPoints = 0;
    float CurrentMeditationDuration = MeditationDuration; // Use component's default duration unless overridden
    USoundCue* MusicToPlay = nullptr;

    switch (MeditationType)
    {
        case EMeditationType::Guided:
            CurrentCompletionPoints = CompletionPoints_Guided;
            CurrentMinimumPoints = MinimumPoints_Guided;
            MusicToPlay = GuidedMeditationMusic;
            UE_LOG(LogTemp, Log, TEXT("Starting Guided Meditation (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentMeditationDuration);
            break;
        case EMeditationType::Mindfulness:
            CurrentCompletionPoints = CompletionPoints_Mindfulness;
            CurrentMinimumPoints = MinimumPoints_Mindfulness;
            MusicToPlay = MindfulnessMeditationMusic;
            UE_LOG(LogTemp, Log, TEXT("Starting Mindfulness Meditation (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentMeditationDuration);
            break;
        case EMeditationType::Cosmic:
            CurrentCompletionPoints = CompletionPoints_Cosmic;
            CurrentMinimumPoints = MinimumPoints_Cosmic;
            MusicToPlay = CosmicMeditationMusic;
            UE_LOG(LogTemp, Log, TEXT("Starting Cosmic Meditation (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentMeditationDuration);
            break;
        case EMeditationType::MeditationPad:
            CurrentCompletionPoints = CompletionPoints_MeditationPad;
            CurrentMinimumPoints = MinimumPoints_MeditationPad;
            MusicToPlay = MeditationPadMusic;
            UE_LOG(LogTemp, Log, TEXT("Starting Meditation Pad (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentMeditationDuration);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown Meditation Type selected!"));
            MusicToPlay = MeditationMusic; // Fallback to general music
            break;
    }

    // Set character state and start the score counter
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Meditating);
    if (BlockRef) { BlockRef->SetMeditationBlockState(EMeditationBlockState::Rising); }

    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CurrentCompletionPoints;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = CurrentMeditationDuration;
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();
        // Immediately update UI
        if (UTimerWidget* TimerWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
        {
            TimerWidget->UpdateTimer(0.0f, TEXT("Time"));
        }
    }
    MeditationTimer = 0.0f; // Reset internal timer

    // Play selected music
    if (MusicToPlay && GetWorld())
    {
        if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying()) { MeditationMusicComponent->Stop(); }
        MeditationMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), MusicToPlay);
        if (MeditationMusicComponent) { MeditationMusicComponent->Play(); }
    }
    else if (!MusicToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("No music assigned for the selected meditation type."));
    }

    // Set timer only if duration is positive
    if (CurrentMeditationDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MeditationTimerHandle,
            this,
            &UMeditationComponent::OnMeditationComplete,
            CurrentMeditationDuration,
            false
        );
    }
}

/**
 *  Stops the meditation process if the current state is Meditating.
 * Clears the timer if applicable and resets to Idle.
 */
void UMeditationComponent::HandleMeditationStop()
{
    if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying())
    {
        MeditationMusicComponent->Stop();
        // Optional: Nullify the pointer if the component is transient and should be garbage collected
        // MeditationMusicComponent = nullptr;
    }
    // Note: State changes (like setting character to Idle) are handled in StopMeditation() or OnMeditationComplete()
}

/**
 *  Called when the timer completes (if `MeditationDuration` > 0).
 * Applies effects and stops meditation.
 */
void UMeditationComponent::OnMeditationComplete()
{


    if (!CachedEscapeCharacter) return;

    // Only proceed if still meditating (guards against race conditions or manual stops)
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        ApplyMeditationEffects();
        StopMeditation(); // Ensure block state is set to Lowering and counter is stopped
    }
}

/**
 *  Applies effects when meditation completes naturally (timer-based only).
 * Placeholder for potential gameplay effects like restoring stamina, applying buffs, etc.
 */
void UMeditationComponent::ApplyMeditationEffects()
{
    if (CachedEscapeCharacter)
    {
        switch (MeditationType)
        {
            case EMeditationType::Guided:
                UE_LOG(LogTemp, Log, TEXT("Guided Meditation Completed"));
                break;
            case EMeditationType::Mindfulness:
                UE_LOG(LogTemp, Log, TEXT("Mindfulness Meditation Completed"));
                break;
            case EMeditationType::Cosmic:
                UE_LOG(LogTemp, Log, TEXT("Cosmic Meditation Completed"));
                break;
            case EMeditationType::MeditationPad:
                UE_LOG(LogTemp, Log, TEXT("Meditation Pad Completed"));
                break;
            default:
                UE_LOG(LogTemp, Log, TEXT("Meditation Completed (Unknown Type)"));
                break;
        }
        // Add general gameplay logic here, e.g., PlayerCharacter->ApplyCalmBuff();
    }
}

/**
 *  Sets the meditation duration in seconds.
 *  @param Duration The new meditation duration in seconds (0 for toggle mode)
 */
void UMeditationComponent::SetMeditationDuration(float Duration)
{
    // Ensure duration is non-negative
    MeditationDuration = FMath::Max(Duration, 0.0f);

    // If meditation is already in progress and timer is active, update the timer
    if (GetWorld() && CachedEscapeCharacter)
    {
        if (CachedEscapeCharacter && CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
        {
            // Clear existing timer
            GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);

            // If new duration is > 0, set a new timer with remaining time
            if (MeditationDuration > 0.0f)
            {
                // Calculate remaining time based on new duration and already elapsed time
                float RemainingTime = FMath::Max(MeditationDuration - MeditationTimer, 0.1f);

                GetWorld()->GetTimerManager().SetTimer(
                    MeditationTimerHandle,
                    this,
                    &UMeditationComponent::OnMeditationComplete,
                    RemainingTime,
                    false
                );
            }
        }
    }
}

/**
 *  Clears the meditation timer, resetting elapsed time to zero.
 *  Useful when stopping meditation prematurely.
 */
void UMeditationComponent::ClearMeditationTimer()
{
    // Reset the internal timer counter
    MeditationTimer = 0.0f;

    // Clear the timer if it's active
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);
    }
}

/**
 *  Checks if the meditation timer is currently active.
 *  @return True if meditation timer is running
 */
bool UMeditationComponent::IsMeditationTimerActive() const
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(MeditationTimerHandle);
}
