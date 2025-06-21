#include "JournalingComponent.h"
#include "../EscapeCharacter.h"
#include "SecondCounterComponent.h"
#include "Kismet/GameplayStatics.h"

UJournalingComponent::UJournalingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UJournalingComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
    if (CachedEscapeCharacter)
    {
        SecondCounterComponent = CachedEscapeCharacter->SecondCounterComponent;
    }
}

void UJournalingComponent::StartJournaling()
{
    if (!CachedEscapeCharacter || !SecondCounterComponent) return;
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() != EMinuteGoalActionsState::Idle)
        return;
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Journaling);
    SecondCounterComponent->CompletionPoints = CompletionPoints;
    SecondCounterComponent->TargetTime = JournalingDuration;
    // You can use MinimumPoints in your logic as needed
    SecondCounterComponent->ResetCounter();
    SecondCounterComponent->StartCounter();
}

void UJournalingComponent::StopJournaling()
{
    if (!CachedEscapeCharacter || !SecondCounterComponent) return;
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Journaling)
    {
        CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);
        SecondCounterComponent->StopCounter();
    }
}
