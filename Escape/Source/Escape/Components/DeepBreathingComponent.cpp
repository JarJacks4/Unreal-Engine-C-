#include "DeepBreathingComponent.h"  
#include "../Widgets/BreathingBubbleWidget.h" 
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "../EscapeCharacter.h"  

UDeepBreathingComponent::UDeepBreathingComponent()  
{  
   PrimaryComponentTick.bCanEverTick = true; // Enable ticking for scale updates  
   BubbleScale = 1.5f; // Initial scale  
   ScaleDirection = -1.0f; // Start shrinking  
   // Cache the owning character for efficiency  
   OwningCharacter = Cast<ACharacter>(GetOwner());  
   DeepBreathingMusicComponent = nullptr; // Initialized as null

}  
// Called every second
void UDeepBreathingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)  
{  
   Super::TickComponent(DeltaTime, TickType, ThisTickFunction);  
   if (Cast<AEscapeCharacter>(OwningCharacter)->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)  
   {  
       UpdateBreathingBubble(DeltaTime);  
   }  
}  
// Called when the player presses the Activity button
void UDeepBreathingComponent::StartDeepBreathing()  
{  
   Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Visible);  
   UBreathingBubbleWidget* BreathingWidget = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetDeepBreathingWidget();  
   if (BreathingWidget)  
   {  
       if (DeepBreathingMusic)
       {
           // Stop any existing music before starting new
           if (DeepBreathingMusicComponent)
           {
               DeepBreathingMusicComponent->Stop();

           }
           // Create and play the music via an audio component
           DeepBreathingMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), DeepBreathingMusic);
           DeepBreathingMusicComponent->Play();
       }
       UButton* TapButton = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetTapButton(); // Gets the tapbutton

       TapButton->SetVisibility(ESlateVisibility::Visible); 
       BubbleScale = 1.5f;  
       BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale)); // Expand  
       BreathingWidget->SetVisibility(ESlateVisibility::Visible);  
       Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetTargetBubble()->SetVisibility(ESlateVisibility::Visible);
       Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterBreathing->ResetCounter();
       Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterBreathing->StartCounter();
       Cast<AEscapeCharacter>(OwningCharacter)->SetMinuteGoalActionState(EMinuteGoalActionsState::DeepBreathing);  
       Cast<AEscapeCharacter>(OwningCharacter)->DeepBreathingComponent->SetDeepBreathingStatus(EDeepBreathingStatus::BreathingIn);  
   }  
}

void UDeepBreathingComponent::StopDeepBreathing()  
{  
    UBreathingBubbleWidget* BreathingWidget = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetDeepBreathingWidget();

   Cast<AEscapeCharacter>(OwningCharacter)->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);
   Cast<AEscapeCharacter>(OwningCharacter)->DeepBreathingComponent->SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
   Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->SetVisibility(ESlateVisibility::Collapsed);

   if (BreathingWidget)  
   {  
       if (DeepBreathingMusicComponent)
       {
           DeepBreathingMusicComponent->Stop();

       }
       UButton* TapButton = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetTapButton();

       TapButton->SetVisibility(ESlateVisibility::Collapsed);
       BreathingWidget->SetVisibility(ESlateVisibility::Collapsed);
       Cast<AEscapeCharacter>(OwningCharacter)->SecondCounterBreathing->StopAndSaveScore();

   }  
}  

void UDeepBreathingComponent::OnBreathingBubbleTapped()
{  
    UBreathingBubbleWidget* BreathingWidget = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetDeepBreathingWidget();
   // Check if tap occurs when bubble is small (perfect timing)  
   if (BubbleScale >= 0.4f && BubbleScale <= 0.6f && ScaleDirection<= 0)  
   {  
       BubbleScale = 1.5f;
       BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale)); // Expand

       Cast<AEscapeCharacter>(OwningCharacter)->DeepBreathingComponent->SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
   }
   else {

       StopDeepBreathing();
   }
}  

void UDeepBreathingComponent::UpdateBreathingBubble(float DeltaTime)  
{  
    UBreathingBubbleWidget* BreathingWidget = Cast<AEscapeCharacter>(OwningCharacter)->GetActivityUIWidget()->GetDeepBreathingWidget();

   BubbleScale += ScaleDirection * DeltaTime * 0.5f; // Adjust scaling speed  
   if (BubbleScale >= 1.5f) 
   {
       ScaleDirection = -1.0f; 
   }
   else if (BubbleScale < 0.5f)
   {
       StopDeepBreathing();
   }

   BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale)); // Shrink  
}

