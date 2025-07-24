// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h" // Include for UButton
#include "GameFramework/Character.h"
#include "BreathingBubbleWidget.generated.h"

class AEscapeCharacter;

class UDeepBreathingComponent; // Forward declare component

/**
 *  UBreathingBubbleWidget
 * Represents the interactive bubble UI element for the deep breathing mini-game.
 * This widget is typically a child of UActivityUIWidget. It contains a button (`BubbleButton`)
 * that the player taps. The widget itself scales up and down, controlled by the UDeepBreathingComponent.
 * It holds a reference to the owning player character.
 */
UCLASS()
class ESCAPE_API UBreathingBubbleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 *  Sets the owning player character reference.
	 *  Player Pointer to the player character (AEscapeCharacter).
	 */
	UFUNCTION(BlueprintCallable, Category = "Breathing Bubble|Setup")
	void SetPlayer(ACharacter* Player) { OwningCharacter = Player; };

	/**
	 *  Gets the owning player character reference.
	 * @return Pointer to the owning ACharacter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Breathing Bubble|Access")
	ACharacter* GetPlayer() const { return OwningCharacter.Get(); }; // Use .Get() for TWeakObjectPtr, added const

	/**
	 *  The interactive button representing the breathing bubble.
	 * Should be assigned either via `meta = (BindWidget)` if named "BubbleButton" in UMG,
	 * or manually via Blueprint/C++. The UDeepBreathingComponent listens for clicks on this button.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing Bubble", meta = (BindWidgetOptional)) // Added BindWidgetOptional
	TObjectPtr<UButton> BubbleButton;

protected:
	/** Optional: Called after the underlying Slate widget is constructed. Good place to bind button delegates. */
	virtual void NativeConstruct() override;

private:
	/**
	 *  Weak pointer reference to the owning player character.
	 * Used to potentially access character state or components (like UDeepBreathingComponent).
	 */
	UPROPERTY(Transient) // Transient as it's set at runtime
	TWeakObjectPtr<ACharacter> OwningCharacter;

	/**  Cached reference to the owning EscapeCharacter for efficient access. */
	AEscapeCharacter* CachedEscapeCharacter = nullptr;

	/**  Internal function bound to the BubbleButton's OnClicked event. */
	UFUNCTION()
	void OnBubbleClicked();
};
