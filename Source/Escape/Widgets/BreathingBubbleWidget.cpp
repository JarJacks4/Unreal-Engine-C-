#include "BreathingBubbleWidget.h"
#include "Components/Button.h"
#include "../EscapeCharacter.h" // Include for casting OwningCharacter
#include "../Components/DeepBreathingComponent.h" // Include for calling OnBreathingBubbleTapped

/**
 *  Called after the underlying Slate widget is constructed.
 * Binds the OnBubbleClicked function to the BubbleButton's OnClicked event.
 */
void UBreathingBubbleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache the cast of OwningCharacter to AEscapeCharacter.
    CachedEscapeCharacter = Cast<AEscapeCharacter>(OwningCharacter.Get());

    // Bind the OnClicked event of the BubbleButton to our local handler function.
    if (BubbleButton)
    {
        BubbleButton->OnClicked.AddDynamic(this, &UBreathingBubbleWidget::OnBubbleClicked);
    }
}

/**
 *  Called when the BubbleButton is clicked by the player.
 * Forwards the tap event to the UDeepBreathingComponent on the owning player character.
 */
void UBreathingBubbleWidget::OnBubbleClicked()
{
    // Ensure the owning player reference is valid.
    if (OwningCharacter.IsValid())
    {
        // Use the cached AEscapeCharacter reference.
        if (CachedEscapeCharacter && CachedEscapeCharacter->DeepBreathingComponent)
        {
            // Call the function on the component to handle the tap logic.
            CachedEscapeCharacter->DeepBreathingComponent->OnBreathingBubbleTapped();
        }
    }
}
