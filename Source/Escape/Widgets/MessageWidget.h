// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MessageWidget.generated.h"


/**
 * 
 */
UCLASS()
class ESCAPE_API UMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetHydrationReminderT(UTextBlock* HydrationReminderTP) { HydrationReminderT = HydrationReminderTP; };

	UFUNCTION(BlueprintCallable)
	UTextBlock* GetHydrationReminderT() { return HydrationReminderT; };

	UFUNCTION(BlueprintCallable, Category = "Wellness Message")
	UTextBlock* GetGratitudeT() { return GratitudeT; };

	UFUNCTION(BlueprintCallable, Category = "Wellness Message")
	UTextBlock* GetAffirmationT() { return AffirmationT; };

	UFUNCTION(BlueprintCallable, Category = "Wellness Message")
	void SetAffirmationT(UTextBlock* AffirmationTP) { AffirmationT = AffirmationTP; };

	UFUNCTION(BlueprintCallable, Category = "Wellness Message")
	void SetGratitudeT(UTextBlock* GratitudeTP) { GratitudeT = GratitudeTP; };

private:
	UTextBlock* HydrationReminderT;


	UTextBlock* GratitudeT;


	UTextBlock* AffirmationT;
};