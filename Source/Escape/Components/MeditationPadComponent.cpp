#include "MeditationPadComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Components/AudioComponent.h"
#include "../Widgets/ActivityUIWidget.h"
#include "../Widgets/BreathingBubbleWidget.h"
#include "../Widgets/ScoreWidget.h"
#include "Components/Image.h"
#include "../Components/SecondCounterComponent.h"

// Constructor: Initializes default values for the component.
UMeditationPadComponent::UMeditationPadComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MeditationMusic = nullptr;
    CachedEscapeCharacter = nullptr;
}

// Called when the component is initialized in the game world. Caches a reference to the owning character.
void UMeditationPadComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
}

// Starts the meditation pad interaction if the current state is Idle. If InteractionDuration > 0, sets a timer; if 0, acts as a toggle.
void UMeditationPadComponent::StartMeditation()
{
    if (!CachedEscapeCharacter) return;
    ClearMeditationTimer();
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Idle) {
        HandleMeditationStart();
    }
}

// Stops the meditation pad interaction if the current state is Interacting. Clears the timer if applicable and resets to Idle.
void UMeditationPadComponent::StopMeditation()
{
    if (!CachedEscapeCharacter) return;
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        HandleMeditationStop();
        if (BlockRef)
        {
            BlockRef->SetMeditationBlockState(EMeditationBlockState::Lowering);
        }
        if (CachedEscapeCharacter->SecondCounterComponent)
        {
            CachedEscapeCharacter->SecondCounterComponent->StopCounter();
        }
    }
}

// Handles the start of interaction: sets state, plays music, and optionally starts a timer. Also manages UI visibility for the interaction activity.
void UMeditationPadComponent::HandleMeditationStart()
{
    if (!CachedEscapeCharacter) return;
    if (UActivityUIWidget* ActivityWidget = CachedEscapeCharacter->GetActivityUIWidget())
    {
        if (UImage* TargetBubble = ActivityWidget->GetTargetBubble())
        {
            TargetBubble->SetVisibility(ESlateVisibility::Hidden);
        }
        if (UBreathingBubbleWidget* BreathingWidget = ActivityWidget->GetDeepBreathingWidget())
        {
            BreathingWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
        ActivityWidget->SetVisibility(ESlateVisibility::Visible);
    }
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Meditating);
    if (BlockRef) BlockRef->SetMeditationBlockState(EMeditationBlockState::Rising);
    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CompletionPoints;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = MeditationDuration;
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();
        if (UScoreWidget* ScoreWidget = CachedEscapeCharacter->SecondCounterComponent->GetScoreWidget())
        {
            ScoreWidget->UpdateActivityProgress(
                CachedEscapeCharacter->SecondCounterComponent->GetElapsedTime(),
                CachedEscapeCharacter->SecondCounterComponent->GetTargetTime(),
                CachedEscapeCharacter->SecondCounterComponent->GetCompletionPoints()
            );
        }
    }
    MeditationTimer = 0.0f;
    if (MeditationMusic && GetWorld())
    {
        if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying())
        {
            MeditationMusicComponent->Stop();
        }
        MeditationMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), MeditationMusic);
        if (MeditationMusicComponent)
        {
            MeditationMusicComponent->Play();
        }
    }
    if (MeditationDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MeditationTimerHandle,
            this,
            &UMeditationPadComponent::OnMeditationComplete,
            MeditationDuration,
            false
        );
    }
}

// Handles the end of interaction: stops music and resets state.
void UMeditationPadComponent::HandleMeditationStop()
{
    if (BlockRef) BlockRef->SetMeditationBlockState(EMeditationBlockState::Lowering);
    if (MeditationMusicComponent && MeditationMusicComponent->IsPlaying())
    {
        MeditationMusicComponent->Stop();
    }
}

// Called when the timer completes (if InteractionDuration > 0). Applies effects and stops interaction.
void UMeditationPadComponent::OnMeditationComplete()
{
    if (!CachedEscapeCharacter) return;
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        ApplyMeditationEffects();
        StopMeditation();
    }
}

// Applies effects when interaction completes naturally (timer-based only). Add gameplay logic here if needed.
void UMeditationPadComponent::ApplyMeditationEffects()
{
    // Add gameplay logic here if needed
}

// Sets the interaction duration in seconds. @param Duration The new interaction duration in seconds (0 for toggle mode)
void UMeditationPadComponent::SetMeditationDuration(float Duration)
{
    MeditationDuration = FMath::Max(Duration, 0.0f);
    if (GetWorld() && CachedEscapeCharacter)
    {
        if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
        {
            GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);
            if (MeditationDuration > 0.0f)
            {
                float RemainingTime = FMath::Max(MeditationDuration - MeditationTimer, 0.1f);
                GetWorld()->GetTimerManager().SetTimer(
                    MeditationTimerHandle,
                    this,
                    &UMeditationPadComponent::OnMeditationComplete,
                    RemainingTime,
                    false
                );
            }
        }
    }
}

// Clears the interaction timer, resetting elapsed time to zero. Useful when stopping interaction prematurely.
void UMeditationPadComponent::ClearMeditationTimer()
{
    MeditationTimer = 0.0f;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);
    }
}

// Checks if the interaction timer is currently active. @return True if interaction timer is running
bool UMeditationPadComponent::IsMeditationTimerActive() const
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(MeditationTimerHandle);
}
