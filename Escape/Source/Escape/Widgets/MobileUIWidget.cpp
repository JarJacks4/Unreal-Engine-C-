

#include "MobileUIWidget.h"
#include "../EscapeCharacter.h"





void UMobileUIWidget::ChangeInteractionUI()
{
	if (InteractionWidget && (PLATFORM_ANDROID || PLATFORM_IOS && OwningCharacter ))
	{

		// Shows the Interaction Widget on the viewport
		InteractionWidget->SetVisibility(ESlateVisibility::Visible);

		switch (Cast<AEscapeCharacter>(OwningCharacter)->GetBlockType()) {
		case EWellnessBlockType::Meditation:
			InteractionWidget->SetWidgetMeditationImage();
			break;
		case EWellnessBlockType::Stretching:
			InteractionWidget->SetWidgetStretchingImage();
			break;
		case EWellnessBlockType::None:
			InteractionWidget->SetWidgetDeepBreathingImage();
			break;
		}
	}
}

