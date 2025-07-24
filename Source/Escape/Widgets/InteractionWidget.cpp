// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionWidget.h"
#include "Components/Button.h"
#include "Styling/SlateBrush.h" // Required for FSlateBrush
#include "Styling/SlateTypes.h" // Required for FButtonStyle
#include "../EscapeCharacter.h" // Required for casting OwningCharacter and calling Activity()


/**
 *  Called after the underlying Slate widget is constructed.
 * Binds the OnInteractionButtonClicked function to the InteractionWidgetButton's OnClicked event.
 */
void UInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind the OnClicked event of the InteractionWidgetButton to our local handler function.
	if (InteractionWidgetButton)
	{
		InteractionWidgetButton->OnClicked.AddDynamic(this, &UInteractionWidget::OnInteractionButtonClicked);
	}
}

/**
 *  Called when the InteractionWidgetButton is clicked by the player.
 * Calls the `Activity()` function on the owning player character.
 */
void UInteractionWidget::OnInteractionButtonClicked()
{
	// Ensure the owning player reference is valid.
	if (OwningCharacter.IsValid())
	{
		// Cast the owning player to the specific character class.
		AEscapeCharacter* PlayerCharacter = Cast<AEscapeCharacter>(OwningCharacter.Get());
		if (PlayerCharacter)
		{
			// Call the character's function to handle starting/stopping the activity.
			PlayerCharacter->Activity();
		}
	}
}

/**
 *  Sets the InteractionWidgetButton's style to use the MeditationImage texture.
 */
void UInteractionWidget::SetWidgetMeditationImage()
{
	ApplyButtonStyle(InteractionWidgetButton, MeditationImage);
}

/**
 *  Sets the InteractionWidgetButton's style to use the StretchingImage texture.
 */
void UInteractionWidget::SetWidgetStretchingImage()
{
	ApplyButtonStyle(InteractionWidgetButton, StretchingImage);
}

/**
 *  Sets the InteractionWidgetButton's style to use the DeepBreathingImage texture.
 */
void UInteractionWidget::SetWidgetDeepBreathingImage()
{
	ApplyButtonStyle(InteractionWidgetButton, DeepBreathingImage);
}

/**
 *  Helper function to apply a texture as the button's style for all states.
 *  Button The button widget to modify.
 *  Texture The texture to apply.
 *  ImageSize The desired size for the image within the button style. Defaults to 200x200.
 */
void UInteractionWidget::ApplyButtonStyle(UButton* Button, UTexture2D* Texture, const FVector2D& ImageSize)
{
	// Ensure both the button and texture are valid before proceeding.
	if (!Button || !Texture)
	{
		return;
	}

	// Create a new Slate Brush using the provided texture.
	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(Texture); // Assign the texture asset.
	NewBrush.ImageSize = ImageSize;      // Set the desired rendering size.
	NewBrush.DrawAs = ESlateBrushDrawType::Image; // Ensure it's drawn as an image.

	// Get the current button style.
	FButtonStyle ButtonStyle = Button->GetStyle();

	// Apply the new brush to all relevant states of the button style.
	ButtonStyle.SetNormal(NewBrush);    // Appearance when idle.
	ButtonStyle.SetHovered(NewBrush);   // Appearance when mouse is over it.
	ButtonStyle.SetPressed(NewBrush);   // Appearance when clicked.
	ButtonStyle.SetDisabled(NewBrush);  // Appearance when disabled (optional, could use a different look).

	// Apply the modified style back to the button.
	Button->SetStyle(ButtonStyle);
}
