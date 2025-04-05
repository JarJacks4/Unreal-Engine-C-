// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/Button.h>
#include "BreathingBubbleWidget.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API UBreathingBubbleWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetPlayer(ACharacter* Player) { OwningPlayer = Player; };

	UFUNCTION(BlueprintCallable)
	ACharacter* GetPlayer() { return OwningPlayer; };


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deep Breathing")
	UButton* BubbleButton;

private:
	ACharacter* OwningPlayer;
};
