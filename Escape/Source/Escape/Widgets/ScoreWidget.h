#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class UTextBlock;

/**
 * UScoreWidget
 * A widget that displays the current score (or seconds count) on screen.
 */
UCLASS()
class ESCAPE_API UScoreWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Function to update the displayed score
    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateScore(float Score);

    UFUNCTION(BlueprintCallable, Category = "Score")
    UTextBlock* GetScoreText() { return ScoreText; };

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetScoreText(UTextBlock* TextBlock) { ScoreText = TextBlock; };

    UFUNCTION(BlueprintCallable, Category = "Player Score")
    ACharacter* GetPlayer() { return OwningCharacter; };

    UFUNCTION(BlueprintCallable, Category = "Player Score")
    void SetPlayer(ACharacter* Player) { OwningCharacter = Player; };


protected:
    virtual void NativeConstruct() override;
    // Reference to the TextBlock widget that displays the score (bind this in UMG)
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreText;

    ACharacter* OwningCharacter;

};
