// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.h"
#include "MobileUIWidget.generated.h"

class ACharacter;
class AEscapeCharacter; // Forward declare specific character class

/**
 *  UMobileUIWidget
 * The main container widget for UI elements specifically designed for mobile platforms.
 * Currently, its primary role is to hold and manage the `InteractionWidget`, which provides
 * a context-sensitive button for triggering wellness activities.
 * It holds a reference to the owning player character to access game state needed for UI updates.
 *
 * This widget is typically created and managed by the AEscapeCharacter on mobile platforms.
 */
UCLASS()
class ESCAPE_API UMobileUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 *  Pointer to the child UInteractionWidget instance.
	 * This widget displays the main interaction button on mobile.
	 * Should be assigned either via `meta = (BindWidget)` if named "InteractionWidget" in UMG,
	 * or manually in Blueprint/C++.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile UI Widget", meta = (BindWidgetOptional)) // Added BindWidgetOptional
	TObjectPtr<UInteractionWidget> InteractionWidget;



	/**
	 *  Sets the owning player character reference.
	 *  Player Pointer to the player character (AEscapeCharacter).
	 */
	UFUNCTION(BlueprintCallable, Category = "Mobile UI Widget|Setup")
	void SetPlayer(ACharacter* Player); // Implementation in .cpp

	/**
	 *  Gets the owning player character reference.
	 * @return Pointer to the owning ACharacter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mobile UI Widget|Access")
	ACharacter* GetPlayer() const { return OwningCharacter.Get(); }; // Use .Get() for TWeakObjectPtr, added const

private:
	/**
	 *  Weak pointer reference to the owning player character.
	 * Used by `ChangeInteractionUI` to get the current interaction context (BlockType).
	 */
	UPROPERTY(Transient) // Transient as it's set at runtime
	TWeakObjectPtr<ACharacter> OwningCharacter;
};
