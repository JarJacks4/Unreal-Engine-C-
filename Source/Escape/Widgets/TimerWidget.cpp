#include "TimerWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Character.h"
#include "Internationalization/Text.h"

void UTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // Optionally hide or reset the timer text on construct
    if (TimerText)
    {
        TimerText->SetText(FText::FromString(TEXT("Time: 0.00")));
    }
}

void UTimerWidget::UpdateTimer(float Time, const FString& Label)
{
    if (TimerText)
    {
        FString TimeString = FString::Printf(TEXT("%s: %.2f"), *Label, Time);
        TimerText->SetText(FText::FromString(TimeString));
    }
}

void UTimerWidget::UpdateActivityTimer(float ElapsedTime, float TargetTime, bool UpdateProgressBar, const FString& Label)
{
    float ClampedElapsed = FMath::Clamp(ElapsedTime, 0.0f, TargetTime);
    float DisplayTime = FMath::RoundToFloat(ClampedElapsed * 100.0f) / 100.0f;
    // Update the timer text
    UpdateTimer(DisplayTime, Label);
    // Update progress bar if requested and available
    if (UpdateProgressBar && TimerProgressBar && TargetTime > 0.0f)
    {
        float Progress = FMath::Clamp(ClampedElapsed / TargetTime, 0.0f, 1.0f);
        TimerProgressBar->SetPercent(Progress);
    }
    // If you want to display points or activity name, add those as separate widgets or parameters here, but do not treat them as timer values.
}
