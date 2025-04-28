// MeditationComponent.cpp
#include "MeditationComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../EscapeCharacter.h"
#include "Components/AudioComponent.h"

UMeditationComponent::UMeditationComponent()
{
    // Disable ticking since we use a timer instead
    PrimaryComponentTick.bCanEverTick = false;
    MeditationMusicComponent = nullptr; // Initialized as null
}

void UMeditationComponent::BeginPlay()
{
    Super::BeginPlay();
    // Cache the owning character for efficiency
    OwningCharacter = Cast<ACharacter>(GetOwner());
}

void UMeditationComponent::StartMeditation()
{
    // Only start if currently idle
    if (Cast<AEscapeCharacter>(OwningCharacter)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Idle) {
        
        HandleMeditationStart();
    }
}

void UMeditationComponent::StopMeditation()
{
     //Only stop if currently meditating
    if (Cast<AEscapeCharacter>(OwningCharacter)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        HandleMeditationStop();
        Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Collapsed);
        Cast<AEscapeCharacter>(OwningCharacter)->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle); 
        Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterMeditation->StopAndSaveScore();
        // Clear timer if it exists (only relevant when duration > 0)
        if (MeditationDuration > 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);
        }
    }
}

void UMeditationComponent::HandleMeditationStart()
{
    Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetTargetBubble()->SetVisibility(ESlateVisibility::Hidden);
    Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetDeepBreathingWidget()->SetVisibility(ESlateVisibility::Collapsed);
    Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Visible);
    Cast<AEscapeCharacter>(OwningCharacter)->SetMinuteGoalActionState(EMinuteGoalActionsState::Meditating);
    Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterMeditation->ResetCounter();
    Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterMeditation->StartCounter();
    MeditationTimer = 0.0f;


    // Play ambient music if assigned
    if (MeditationMusic)
    {
        // Stop any existing music before starting new
        if (MeditationMusicComponent)
        {
            MeditationMusicComponent->Stop();
           
        }
        // Create and play the music via an audio component
        MeditationMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), MeditationMusic);
        MeditationMusicComponent->Play();
    }

    // Set timer only if duration is positive
    if (MeditationDuration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            MeditationTimerHandle,
            this,
            &UMeditationComponent::OnMeditationComplete,
            MeditationDuration,
            false // Single-shot timer
        );
    }
}

void UMeditationComponent::HandleMeditationStop()
{


    // Stop meditation music if playing
    if (MeditationMusicComponent)
    {
        MeditationMusicComponent->Stop();
        MeditationMusicComponent = nullptr; // Clear reference after stopping
    }
}

void UMeditationComponent::OnMeditationComplete()
{
    // Only proceed if still meditating (guards against race conditions)
    if (Cast<AEscapeCharacter>(OwningCharacter)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        ApplyMeditationEffects();
        HandleMeditationStop();    
    }
}

void UMeditationComponent::ApplyMeditationEffects()
{
    if (OwningCharacter.IsValid())
    {

    }
}