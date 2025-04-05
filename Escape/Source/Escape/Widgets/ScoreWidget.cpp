#include "ScoreWidget.h"
#include "Components/TextBlock.h"

void UScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();

	// Hides the score widget at the start
    if (ScoreText)
    {        
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UScoreWidget::UpdateScore(float Score)
{
    if (ScoreText)
    {
        // Format the score as a string (e.g., "Score: 12.34")
        FString ScoreString = FString::Printf(TEXT("Score: %.2f"), Score);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
}
