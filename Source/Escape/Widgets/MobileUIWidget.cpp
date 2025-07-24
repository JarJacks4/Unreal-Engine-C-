#include "MobileUIWidget.h"
#include "../EscapeCharacter.h" // Include for casting OwningCharacter and getting BlockType
#include "InteractionWidget.h" // Ensure InteractionWidget definition is included
#include "Kismet/GameplayStatics.h" // For GetPlatformName
#include "../Components/WellnessComponent.h"

void UMobileUIWidget::SetPlayer(ACharacter* Player)
{
	OwningCharacter = TWeakObjectPtr<ACharacter>(Player); 
}
