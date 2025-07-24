#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h" // For UTexture2D
#include "Components/Button.h" // For UButton
#include "GameFramework/Character.h"
#include "InteractionWidget.generated.h"

/**
 *  UInteractionWidget
 * A widget designed for mobile platforms to display a single interaction button.
 * The button's appearance changes based on the type of wellness activity available
 * (Meditation, Deep Breathing, Stretching) using different textures.
 * It holds a reference to the owning player character and likely triggers the character's
 * `Activity()` function when clicked.
 */
UCLASS()
class ESCAPE_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 *  Sets the InteractionWidgetButton's style to use the MeditationImage texture.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget|Appearance")
	void SetWidgetMeditationImage();

	/**
	 *  Sets the InteractionWidgetButton's style to use the DeepBreathingImage texture.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget|Appearance")
	void SetWidgetDeepBreathingImage();

	/**
	 *  Sets the InteractionWidgetButton's style to use the StretchingImage texture.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget|Appearance")
	void SetWidgetStretchingImage();

	/**
	 *  Sets the owning player character reference.
	 *  Player Pointer to the player character (AEscapeCharacter).
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget|Setup")
	void SetPlayer(ACharacter* Player) { OwningCharacter = Player; };
	/**
	 *  Gets the owning player character reference.
	 * @return Pointer to the owning ACharacter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction Widget|Access")
	ACharacter* GetPlayer() const { return OwningCharacter.Get(); }; // Use .Get() for TWeakObjectPtr, added const

	/**
	 *  The main interactive button for triggering wellness activities on mobile.
	 * Should be assigned via `meta = (BindWidget)` if named "InteractionWidgetButton" in UMG.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget", meta = (BindWidgetOptional)) // Added BindWidgetOptional
	TObjectPtr<UButton> InteractionWidgetButton;

	/**
	 *  Texture asset used for the button's appearance when the Meditation activity is available.
	 * Assign this texture in the Blueprint Editor for this widget class or instance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget|Appearance")
	TObjectPtr<UTexture2D> MeditationImage;

	/**
	 *  Texture asset used for the button's appearance when the Deep Breathing activity is available (or as default).
	 * Assign this texture in the Blueprint Editor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget|Appearance")
	TObjectPtr<UTexture2D> DeepBreathingImage;

	/**
	 *  Texture asset used for the button's appearance when the Stretching activity is available.
	 * Assign this texture in the Blueprint Editor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Widget|Appearance")
	TObjectPtr<UTexture2D> StretchingImage;

protected:
	/** Optional: Called after the underlying Slate widget is constructed. Good place to bind button delegates. */
	virtual void NativeConstruct() override;

private:
	/**
	 *  Weak pointer reference to the owning player character.
	 * Used to call the `Activity()` function when the button is clicked.
	 */
	UPROPERTY(Transient) // Transient as it's set at runtime
	TWeakObjectPtr<ACharacter> OwningCharacter;

	/**  Internal function bound to the InteractionWidgetButton's OnClicked event. */
	UFUNCTION()
	void OnInteractionButtonClicked();

	/**
	 *  Helper function to apply a texture as the button's style for all states (Normal, Hovered, Pressed, Disabled).
	 *  Button The button widget to modify.
	 *  Texture The texture to apply.
	 *  ImageSize The desired size for the image within the button style.
	 */
	void ApplyButtonStyle(UButton* Button, UTexture2D* Texture, const FVector2D& ImageSize = FVector2D(200, 200));
};
