// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionWidget.h"
#include "BreathingBubbleWidget.h"
#include "MessageWidget.h"
#include "Blueprint/UserWidget.h"
#include "MobileUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API UMobileUIWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * Variable to store the interaction widget
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile UI Widget")
	UInteractionWidget* InteractionWidget;

	/**
	* Displays the interaction UI for the specified wellness block type.
	* @param BlockType The type of wellness block (Stretching, Deep Breathing or Meditation).
	*/
	UFUNCTION(BlueprintCallable)
	void ChangeInteractionUI();

	UFUNCTION(BlueprintCallable)
	void SetPlayer(ACharacter* Player) { OwningCharacter = Player; };
	UFUNCTION(BlueprintCallable)
	ACharacter* GetPlayer() { return OwningCharacter; };

private:

	// cached reference of the owning character
	ACharacter* OwningCharacter;
};
