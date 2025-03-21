// MeditationComponent.cpp
#include "MeditationComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EscapeCharacter.h"
#include "Components/AudioComponent.h"

UMeditationComponent::UMeditationComponent()
{
    // Disable ticking since we use a timer instead
    PrimaryComponentTick.bCanEverTick = false;
    MeditationState = EMeditationState::Idle;
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
    if (MeditationState == EMeditationState::Idle)
    {
        HandleMeditationStart();
    }
}
void UMeditationComponent::Meditation()
{
    if (GetMeditationState() == EMeditationState::Idle) {
        // Call the meditation component’s start function
        StartMeditation();
        // Note: For Mobile, this is typically triggered by a Blueprint UI button rather than keyboard input
    }
    else {
        StopMeditation();
    }
}
void UMeditationComponent::StopMeditation()
{
    // Only stop if currently meditating
    if (MeditationState == EMeditationState::Meditating)
    {
        HandleMeditationStop();
        MeditationState = EMeditationState::Idle; // Always reset to Idle for simplicity

        // Clear timer if it exists (only relevant when duration > 0)
        if (MeditationDuration > 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(MeditationTimerHandle);
        }
    }
}

void UMeditationComponent::HandleMeditationStart()
{
    MeditationState = EMeditationState::Meditating;
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
    if (MeditationState == EMeditationState::Meditating)
    {
        ApplyMeditationEffects();
        HandleMeditationStop();
        MeditationState = EMeditationState::Idle;
    }
}

void UMeditationComponent::ApplyMeditationEffects()
{
    // Apply gameplay effects when meditation completes naturally
    if (OwningCharacter.IsValid())
    {

    }
}