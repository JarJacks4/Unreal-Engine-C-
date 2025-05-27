#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// Include necessary headers for member types
#include "Components/Image.h" // For UImage
#include "Components/Button.h" // For UButton
#include "ScoreWidget.h" // For UScoreWidget
#include "BreathingBubbleWidget.h" // For UBreathingBubbleWidget
#include "TimerWidget.h" // Add include for TimerWidget
#include "ActivityUIWidget.generated.h"

/**
 *  UActivityUIWidget
 * A container widget that holds and manages the UI elements specific to the active wellness activity.
 * This includes the score display (UScoreWidget), the deep breathing bubble (UBreathingBubbleWidget),
 * the tap button for deep breathing, the target bubble visual for deep breathing, and the timer display (UTimerWidget).
 * It provides accessors for these child widgets so the character and activity components can interact with them.
 *
 * This widget is typically created and managed by the AEscapeCharacter and made visible/hidden
 * when an activity starts or stops.
 */
UCLASS()
class ESCAPE_API UActivityUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Accessors and Mutators for Child Widgets ---

	/**
	 *  Sets the reference to the UScoreWidget instance used within this activity UI.
	 *  ScoreWidgetP Pointer to the UScoreWidget instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Activity UI|Setup") // Added category
	void SetScoreWidget(UScoreWidget* ScoreWidgetP) { ScoreWidget = ScoreWidgetP; };

	/**
	 *  Gets the reference to the UScoreWidget instance used for displaying the score/time.
	 * @return Pointer to the UScoreWidget, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity UI|Access") // Added category and BlueprintPure
	UScoreWidget* GetScoreWidget() const { return ScoreWidget; }; // Added const


	
	/**
	 *  Sets the reference to the UBreathingBubbleWidget instance used for the deep breathing activity.
	 *  DeepBreathin Pointer to the UBreathingBubbleWidget instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Activity UI|Setup") // Added category
	void SetDeepBreathingWidget(UBreathingBubbleWidget* DeepBreathin) { BreathingWidget = DeepBreathin; };

	/**
	 *  Gets the reference to the UBreathingBubbleWidget instance used for the deep breathing activity.
	 * @return Pointer to the UBreathingBubbleWidget, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity UI|Access") // Added category and BlueprintPure
	UBreathingBubbleWidget* GetDeepBreathingWidget() const { return BreathingWidget; }; // Added const

	/**
	 *  Gets the reference to the UButton instance used for tapping during the deep breathing activity.
	 * @return Pointer to the UButton, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity UI|Access|Deep Breathing") // Added category and BlueprintPure
	UButton* GetTapButton() const { return TapButton; }; // Added const

	/**
	 *  Gets the reference to the UImage instance representing the target zone/visual for the deep breathing activity.
	 * @return Pointer to the UImage, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity UI|Access|Deep Breathing") // Added category and BlueprintPure
	UImage* GetTargetBubble() const { return TargetBubble; }; // Added const

	/**
	 *  Sets the reference to the UButton instance used for tapping during the deep breathing activity.
	 *  TapButtonP Pointer to the UButton instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Activity UI|Setup") // Added category
	void SetTapButton(UButton* TapButtonP) { TapButton = TapButtonP; };

	/**
	 *  Sets the reference to the UImage instance representing the target zone/visual for the deep breathing activity.
	 *  TargetBubbleP Pointer to the UImage instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Activity UI|Setup") // Added category
	void SetTargetBubble(UImage* TargetBubbleP) { TargetBubble = TargetBubbleP; };

	/**
	 *  Sets the reference to the UTimerWidget instance used for displaying the timer.
	 *  TimerWidgetP Pointer to the UTimerWidget instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Activity UI|Setup")
	void SetTimerWidget(UTimerWidget* TimerWidgetP) { TimerWidget = TimerWidgetP; };

	/**
	 *  Gets the reference to the UTimerWidget instance used for displaying the timer.
	 * @return Pointer to the UTimerWidget, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity UI|Access")
	UTimerWidget* GetTimerWidget() const { return TimerWidget; };

private:
	/**
	 *  Pointer to the child UBreathingBubbleWidget instance.
	 * Should be assigned either via `meta = (BindWidget)` if named identically in UMG, or manually via `SetDeepBreathingWidget`.
	 */
	UPROPERTY(meta = (BindWidgetOptional)) // Use BindWidgetOptional if it might not exist in all UMG layouts using this class
	TObjectPtr<UBreathingBubbleWidget> BreathingWidget;

	/**
	 *  Pointer to the child UScoreWidget instance.
	 * Should be assigned either via `meta = (BindWidget)` or manually via `SetScoreWidget`.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScoreWidget> ScoreWidget;

	/**
	 *  Pointer to the child UButton instance used for deep breathing taps.
	 * Should be assigned either via `meta = (BindWidget)` or manually via `SetTapButton`.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TapButton;

	/**
	 *  Pointer to the child UImage instance representing the deep breathing target.
	 * Should be assigned either via `meta = (BindWidget)` or manually via `SetTargetBubble`.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> TargetBubble;

	/**
	 *  Pointer to the child UTimerWidget instance.
	 * Should be assigned either via `meta = (BindWidget)` or manually via `SetTimerWidget`.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTimerWidget> TimerWidget;
};
