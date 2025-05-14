#include "ScoreWidget.h"
#include "Components/TextBlock.h" // Include for UTextBlock
#include "GameFramework/Character.h"
#include "Internationalization/Text.h" // Include for FText formatting
#include "Components/ProgressBar.h" // Include for UProgressBar

/**
 *  Called after the underlying Slate widget is constructed.
 * Hides the score widget by default when it's first created.
 * It should be made visible by the component that starts tracking the score (e.g., USecondCounterComponent::StartCounter).
 */
void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
}
void UScoreWidget::SetPlayer(ACharacter* Player)
{
    OwningCharacter = Player;
}
/**
 *  Updates the text displayed in the ScoreText widget.
 * Formats the input float value to two decimal places, prefixed with "Score: ".
 * Handles cases where ScoreText might be null.
 *  Score The numerical score or time value to display.
 */
void UScoreWidget::UpdateScore(float Score, const FString& Label)
{
    if (ScoreText)
    {
        FNumberFormattingOptions FormatOptions = FNumberFormattingOptions::DefaultWithGrouping();
        FormatOptions.SetMaximumFractionalDigits(2);
        FormatOptions.SetMinimumFractionalDigits(2);

        FText ScoreFormattedText = FText::Format(
            FText::FromString(Label + TEXT(": {0}")),
            FText::AsNumber(Score, &FormatOptions)
        );
        ScoreText->SetText(ScoreFormattedText);
    }
}

/**
 *  Updates the widget with complete activity information.
 *  Displays elapsed time, remaining time, progress, and points based on current settings.
 *  @param ElapsedTime Current time elapsed in the activity
 *  @param TargetTime Total time required for activity completion
 *  @param Points Points awarded upon completion
 *  @param UpdateProgressBar Whether to update the progress bar (if available)
 */
void UScoreWidget::UpdateActivityProgress(float ElapsedTime, float TargetTime, int32 Points, bool UpdateProgressBar)
{
    float ClampedElapsed = FMath::Clamp(ElapsedTime, 0.0f, TargetTime);
    float DisplayTime = FMath::RoundToFloat(ClampedElapsed * 100.0f) / 100.0f;
    CurrentTargetTime = TargetTime;
    CurrentPoints = Points;
    
    // Update the basic score text (legacy support)
    UpdateScore(DisplayTime, TEXT("Score"));
    
    // Update the time text based on display mode
    if (TimeText)
    {
        float TimeToDisplay = bShowTimeRemaining ? FMath::Max(0.0f, TargetTime - DisplayTime) : DisplayTime;
        TimeText->SetText(FText::FromString(FormatTime(TimeToDisplay)));
    }
    
    // Update the points text if available
    if (PointsText)
    {
        PointsText->SetText(FText::Format(
            NSLOCTEXT("ScoreWidget", "PointsFormat", "{0} points"),
            FText::AsNumber(Points)
        ));
    }
    
    // Update the activity name if available
    if (ActivityNameText && !CurrentActivityName.IsEmpty())
    {
        ActivityNameText->SetText(FText::FromString(CurrentActivityName));
    }
    
    // Update progress bar if requested and available
    if (UpdateProgressBar && ActivityProgressBar && TargetTime > 0.0f)
    {
        float Progress = FMath::Clamp(ClampedElapsed / TargetTime, 0.0f, 1.0f);
        ActivityProgressBar->SetPercent(Progress);
    }
}

/**
 *  Sets the display mode for the widget
 *  @param bShowTimeRemainingParam If true, shows time remaining; if false, shows elapsed time
 */
void UScoreWidget::SetTimeRemainingMode(bool bShowTimeRemainingParam)
{
    bShowTimeRemaining = bShowTimeRemainingParam;
}

/**
 *  Sets the activity name to be displayed
 *  @param ActivityName Name of the current activity (e.g., "Guided Meditation", "Deep Breathing")
 */
void UScoreWidget::SetActivityName(const FString& ActivityName)
{
    CurrentActivityName = ActivityName;
    
    if (ActivityNameText)
    {
        ActivityNameText->SetText(FText::FromString(CurrentActivityName));
    }
}

/**
 *  Formats time in seconds to a readable string (MM:SS or MM:SS.MS format)
 *  @param TimeInSeconds Time to format
 *  @param bShowMilliseconds Whether to include milliseconds
 *  @return Formatted time string
 */
FString UScoreWidget::FormatTime(float TimeInSeconds, bool bShowMilliseconds)
{
    // Calculate minutes and seconds
    int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
    int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeInSeconds, 60.0f));
    
    if (bShowMilliseconds)
    {
        // Calculate milliseconds (first two digits only)
        int32 Milliseconds = FMath::FloorToInt(FMath::Fmod(TimeInSeconds * 100.0f, 100.0f));
        return FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, Seconds, Milliseconds);
    }
    else
    {
        return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    }
}

/**
 *  Animates the score to a target value smoothly.
 *  @param NewScore The target score value.
 *  @param Speed The speed of the animation.
 */
void UScoreWidget::AnimateScoreTo(float NewScore, float Speed)
{
    AnimatedScoreTarget = NewScore;
    AnimatedScoreSpeed = Speed;
    bIsAnimatingScore = true;
    // Optionally, set AnimatedScoreCurrent to the current displayed value if first time
}

/**
 *  Called every frame to update the widget.
 *  Handles smooth score animation.
 *  @param MyGeometry The geometry of the widget.
 *  @param InDeltaTime Time elapsed since the last frame.
 */
void UScoreWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (bIsAnimatingScore)
    {
        if (FMath::IsNearlyEqual(AnimatedScoreCurrent, AnimatedScoreTarget, 0.01f))
        {
            AnimatedScoreCurrent = AnimatedScoreTarget;
            bIsAnimatingScore = false;
        }
        else
        {
            AnimatedScoreCurrent = FMath::FInterpConstantTo(AnimatedScoreCurrent, AnimatedScoreTarget, InDeltaTime, AnimatedScoreSpeed);
        }
        UpdateScore(AnimatedScoreCurrent, TEXT("Score"));
    }
}
