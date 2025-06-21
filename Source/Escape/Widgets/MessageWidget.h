// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h" // Include for UTextBlock
#include "MessageWidget.generated.h"

/**
 *  UMessageWidget
 * A UI widget dedicated to displaying various wellness-related text messages to the player.
 * It contains separate UTextBlock elements for hydration reminders, gratitude prompts, and affirmations.
 * The visibility and content of these text blocks are controlled by the UWellnessComponent.
 *
 * This widget is typically created and managed by the UWellnessComponent and added to the viewport.
 */
UCLASS()
class ESCAPE_API UMessageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // --- Accessors and Mutators for TextBlocks ---
    // Note: Using BindWidget is generally preferred over manual Set/Get if the widget hierarchy is stable.

    /**
     *  Sets the UTextBlock instance used for displaying hydration reminders.
     *  HydrationReminderTP Pointer to the UTextBlock widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Wellness Message|Setup")
    void SetHydrationReminderT(UTextBlock* HydrationReminderTP) { HydrationReminderT = HydrationReminderTP; };

    /**
     *  Gets the UTextBlock instance used for hydration reminders.
     * @return Pointer to the UTextBlock, or nullptr if not set/bound.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Wellness Message|Access")
    UTextBlock* GetHydrationReminderT() const { return HydrationReminderT; }; // Added const

    /**
     *  Gets the UTextBlock instance used for gratitude messages.
     * @return Pointer to the UTextBlock, or nullptr if not set/bound.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Wellness Message|Access")
    UTextBlock* GetGratitudeT() const { return GratitudeT; }; // Added const

    /**
     *  Gets the UTextBlock instance used for affirmations.
     * @return Pointer to the UTextBlock, or nullptr if not set/bound.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Wellness Message|Access")
    UTextBlock* GetAffirmationT() const { return AffirmationT; }; // Added const

    /**
     *  Sets the UTextBlock instance used for displaying affirmations.
     *  AffirmationTP Pointer to the UTextBlock widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Wellness Message|Setup")
    void SetAffirmationT(UTextBlock* AffirmationTP) { AffirmationT = AffirmationTP; };

    /**
     *  Sets the UTextBlock instance used for displaying gratitude messages.
     *  GratitudeTP Pointer to the UTextBlock widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Wellness Message|Setup")
    void SetGratitudeT(UTextBlock* GratitudeTP) { GratitudeT = GratitudeTP; };

private:
    /**
     *  TextBlock for displaying hydration reminders.
     * Should be assigned either via `meta = (BindWidget)` if named "HydrationReminderT" in UMG,
     * or manually via `SetHydrationReminderT`.
     */
    UPROPERTY(meta = (BindWidgetOptional)) // Use BindWidgetOptional
    TObjectPtr<UTextBlock> HydrationReminderT;

    /**
     *  TextBlock for displaying gratitude messages.
     * Should be assigned either via `meta = (BindWidget)` if named "GratitudeT" in UMG,
     * or manually via `SetGratitudeT`.
     */
    UPROPERTY(meta = (BindWidgetOptional)) // Use BindWidgetOptional
    TObjectPtr<UTextBlock> GratitudeT;

    /**
     *  TextBlock for displaying affirmations.
     * Should be assigned either via `meta = (BindWidget)` if named "AffirmationT" in UMG,
     * or manually via `SetAffirmationT`.
     */
    UPROPERTY(meta = (BindWidgetOptional)) // Use BindWidgetOptional
    TObjectPtr<UTextBlock> AffirmationT;
};
