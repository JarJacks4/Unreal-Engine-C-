#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ACharacter;

/**
 *  UTimerWidget
 *  A UI widget dedicated to displaying a timer (elapsed or remaining time) for wellness activities.
 *  Contains a UTextBlock element (`TimerText`) which is updated via the `UpdateTimer` function.
 *  This widget is intended to be used alongside UScoreWidget for activities that require both a timer and a score display.
 */
UCLASS()
class ESCAPE_API UTimerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     *  Updates the text displayed in the TimerText widget.
     *  Formats the input float value to two decimal places, prefixed with "Time: ".
     *  Handles cases where TimerText might be null.
     *  @param Time The numerical time value to display.
     *  @param Label Optional label to prefix (default: "Time")
     */
    UFUNCTION(BlueprintCallable, Category = "Activity")
    void UpdateTimer(float Time, const FString& Label = TEXT("Time"));

    /**
     *  Updates the timer widget with complete activity information.
     *  Displays elapsed/remaining time, progress bar, and optionally a label.
     *  @param ElapsedTime Current time elapsed in the activity
     *  @param TargetTime Total time required for activity completion
     *  @param UpdateProgressBar Whether to update the progress bar (if available)
     *  @param Label Optional label for the timer (e.g., "Time")
     */
    UFUNCTION(BlueprintCallable, Category = "Activity")
    void UpdateActivityTimer(float ElapsedTime, float TargetTime, bool UpdateProgressBar = true, const FString& Label = TEXT("Time"));

protected:
    virtual void NativeConstruct() override;

private:
    /**
     *  TextBlock for displaying the timer value.
     *  Should be assigned via `meta = (BindWidget)` if named "TimerText" in UMG, or manually via Blueprint/C++.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TimerText;

    /**
     *  ProgressBar for displaying the timer progress.
     *  Should be assigned via `meta = (BindWidgetOptional)` if named "TimerProgressBar" in UMG, or manually via Blueprint/C++.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> TimerProgressBar;
};
