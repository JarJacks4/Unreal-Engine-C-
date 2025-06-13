#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

// Forward declarations
class UTextBlock;
class ACharacter;

/**
 *  UScoreWidget
 * A UI widget dedicated to displaying activity progress, points, and optionally a progress bar.
 * It contains a UTextBlock element (`ScoreText`) which is updated via the `UpdateScore` function.
 * This widget is typically used for wellness activities like meditation, breathing exercises, etc.
 *
 * Note: Timer display is now handled by UTimerWidget.
 */
UCLASS()
class ESCAPE_API UScoreWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     *  Updates the text displayed in the ScoreText widget.
     * Formats the input float value to two decimal places prefixed with "Score: ".
     *  Score The numerical score or time value to display.
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateScore(float Score, const FString& Label = TEXT("Score"));
    


    /**
     *  Gets the UTextBlock instance used for displaying the score.
     * @return Pointer to the UTextBlock, or nullptr if not set/bound.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score|Access")
    UTextBlock* GetScoreText() const { return ScoreText; }; // Added const

    /**
     *  Sets the UTextBlock instance used for displaying the score.
     * Useful if not using `meta = (BindWidget)`.
     *  TextBlock Pointer to the UTextBlock widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Score|Setup")
    void SetScoreText(UTextBlock* TextBlock) { ScoreText = TextBlock; };

    /**
     *  Gets the owning player character reference (if set).
     * @return Pointer to the owning ACharacter, or nullptr.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score|Access")
    ACharacter* GetPlayer() const { return OwningCharacter.Get(); }; // Use .Get(), added const

    /**
     *  Sets the owning player character reference.
     *  Player Pointer to the player character.
     */
    UFUNCTION(BlueprintCallable, Category = "Score|Setup")
    void SetPlayer(ACharacter* Player);

    /**
     *  Animates the score display to a target value.
     *  NewScore The target score value to animate to.
     *  Speed The speed of the animation.
     */
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AnimateScoreTo(float NewScore, float Speed = 200.0f);

protected:
    /**
     *  Called after the underlying Slate widget is constructed.
     * Initializes the widget, typically setting its initial visibility to Collapsed.
     */
    virtual void NativeConstruct() override;

    /**
     *  Called every frame to update the widget.
     * Handles score animation logic.
     */
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /**
     *  Reference to the TextBlock widget that displays the score.
     * Should be assigned either via `meta = (BindWidget)` if named "ScoreText" in UMG,
     * or manually via `SetScoreText`.
     */
    UPROPERTY(meta = (BindWidgetOptional)) // Use BindWidgetOptional
    TObjectPtr<UTextBlock> ScoreText;
    
    /**
     *  Reference to the TextBlock widget that displays the activity name.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ActivityNameText;
    
    /**
     *  Reference to the TextBlock widget that displays points to be awarded.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PointsText;

    /**
     *  Weak pointer reference to the owning player character.
     * May be used for context if needed, but currently unused in this widget's logic.
     */
    UPROPERTY(Transient) // Transient as it's set at runtime
    TWeakObjectPtr<ACharacter> OwningCharacter;

private:
    /**
     *  Current animated score value.
     */
    float AnimatedScoreCurrent = 0.0f;

    /**
     *  Target animated score value.
     */
    float AnimatedScoreTarget = 0.0f;

    /**
     *  Speed of the score animation, editable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score Animation", meta = (AllowPrivateAccess = "true"))
    float AnimatedScoreSpeed = 200.0f;

    /**
     *  Whether the score is currently animating.
     */
    bool bIsAnimatingScore = false;
};
