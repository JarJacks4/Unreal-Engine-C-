#include "ScoreWidget.h"
#include "Components/TextBlock.h" // Include for UTextBlock
#include "GameFramework/Character.h"
#include "Internationalization/Text.h" // Include for FText formatting

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
 *  Note: Timer display is now handled by UTimerWidget.
 */
void UScoreWidget::UpdateScore(float Score, const FString& Label)
{
    if (ScoreText)
    {
        FString ScoreString = FString::Printf(TEXT("%s: %.2f"), *Label, Score);
        ScoreText->SetText(FText::FromString(ScoreString));
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

// Note: Do not display timer here. Use UTimerWidget for timer display.
