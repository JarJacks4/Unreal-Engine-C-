#include "StretchingComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../EscapeCharacter.h"

// Sets default values for this component's properties
UStretchingComponent::UStretchingComponent()
{
	//  Tick disabled to gain performance
	PrimaryComponentTick.bCanEverTick = false;
	StretchingMusicComponent = nullptr; // Initialized as null

}

void UStretchingComponent::StartStretching()
{
	if (StretchingMusic)
	{
		// Stop any existing music before starting new
		if (StretchingMusicComponent)
		{
			StretchingMusicComponent->Stop();

		}
		// Create and play the music via an audio component
		StretchingMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), StretchingMusic);
		StretchingMusicComponent->Play();
	}
	AEscapeCharacter* Character = Cast<AEscapeCharacter>(OwningCharacter);
	Character->SecondCounterStretching->ResetCounter();
	Character->SecondCounterStretching->StartCounter();
	Character->GetActivityUIWidget()->GetTargetBubble()->SetVisibility(ESlateVisibility::Hidden);
	Character->GetActivityUIWidget()->GetDeepBreathingWidget()->SetVisibility(ESlateVisibility::Collapsed);
	Character->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Visible);
	Character->SetMinuteGoalActionState(EMinuteGoalActionsState::Stretching);
}

void UStretchingComponent::StopStretching()
{
	if (StretchingMusicComponent)
	{
		StretchingMusicComponent->Stop();

	}
	AEscapeCharacter* Character = Cast<AEscapeCharacter>(OwningCharacter);
	Character->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Collapsed);

	Character->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);
	Character->SecondCounterStretching->StopAndSaveScore();


}

// Called when the game starts
void UStretchingComponent::BeginPlay()
{
	Super::BeginPlay();
	// Cache the owning character for efficiency
	OwningCharacter = Cast<ACharacter>(GetOwner());
	
}


// Called every frame
void UStretchingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

