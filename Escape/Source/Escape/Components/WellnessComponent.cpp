#include "WellnessComponent.h"
#include "../Widgets/MessageWidget.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../WellnessSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"


void UWellnessComponent::ShowGratitudePrompt()
{
    MessageWidget->GetGratitudeT()->SetVisibility(ESlateVisibility::Visible);

    MessageWidget->GetGratitudeT()->SetText(GratitudeFT);

    GetWorld()->GetTimerManager().SetTimer(GratitudeRemovalTimerHandle, [this]() {
        MessageWidget->GetGratitudeT()->SetVisibility(ESlateVisibility::Collapsed);
        }, 10.0f, false);



}


void UWellnessComponent::BeginPlay()
{
    Super::BeginPlay();
    MessageWidget = CreateWidget<UMessageWidget>(GetWorld(), MessageWidgetClass);
    MessageWidget->AddToViewport();




	// Sets repeating timers for hydration, affirmation, and gratitude prompts
    GetWorld()->GetTimerManager().SetTimer(AffirmationTimerHandle, this, &UWellnessComponent::ShowAffirmationReminder, AffirmationInterval, true);
    GetWorld()->GetTimerManager().SetTimer(HydrationTimerHandle, this, &UWellnessComponent::ShowHydrationReminder, HydrationInterval, true);
    GetWorld()->GetTimerManager().SetTimer(GratitudeTimerHandle, this, &UWellnessComponent::ShowGratitudePrompt, GratitudeInterval, true);


}



void UWellnessComponent::ShowHydrationReminder()
{
    MessageWidget->GetHydrationReminderT()->SetVisibility(ESlateVisibility::Visible);
	MessageWidget->GetHydrationReminderT()->SetText(HydrationReminderFT);
    // Schedule the widget to be removed after 10 seconds using a timer
    GetWorld()->GetTimerManager().SetTimer(HydrationRemovalTimerHandle, [this]() {
        MessageWidget->GetHydrationReminderT()->SetVisibility(ESlateVisibility::Collapsed);
        }, 10.0f, false);
}


void UWellnessComponent::ShowAffirmationReminder()
{
    if (Affirmations.Num() > 0)
    {
        FMessage RandomAffirmation = Affirmations[FMath::RandRange(0, Affirmations.Num() - 1)];
        MessageWidget->GetAffirmationT()->SetVisibility(ESlateVisibility::Visible);
        MessageWidget->GetAffirmationT()->SetText(RandomAffirmation.Text);        // Play voice clip if enabled and available
        if (bEnableAffirmationVoice && RandomAffirmation.VoiceClip)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), RandomAffirmation.VoiceClip);
        }
        GetWorld()->GetTimerManager().SetTimer(AffirmationRemovalTimerHandle, [this]() {
            MessageWidget->GetAffirmationT()->SetVisibility(ESlateVisibility::Collapsed);
            }, 10.0f, false);

    }


}

