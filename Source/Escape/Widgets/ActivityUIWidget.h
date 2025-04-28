
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/Image.h>
#include "../Widgets/ScoreWidget.h" 
#include "../Widgets/BreathingBubbleWidget.h" 
#include "ActivityUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPE_API UActivityUIWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetScoreWidget(UScoreWidget* ScoreWidgetP) { ScoreWidget = ScoreWidgetP; };

	UFUNCTION(BlueprintCallable)
	UScoreWidget* GetScoreWidget() { return ScoreWidget; };

	UFUNCTION(BlueprintCallable)
	void SetDeepBreathingWidget(UBreathingBubbleWidget* DeepBreathin) { BreathingWidget = DeepBreathin; };

	UFUNCTION(BlueprintCallable)
	UBreathingBubbleWidget* GetDeepBreathingWidget() { return BreathingWidget; };

	UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
	UButton* GetTapButton() {	return TapButton;};

	UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
	UImage* GetTargetBubble() {return TargetBubble;};

	UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
	void SetTapButton(UButton* TapButtonP) { TapButton = TapButtonP; };

	UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
	void SetTargetBubble(UImage* TargetBubbleP) { TargetBubble = TargetBubbleP; };

private:

	// The active breathing bubble widget instance  
	UBreathingBubbleWidget* BreathingWidget;
	UScoreWidget* ScoreWidget;
	UButton* TapButton;

	UImage* TargetBubble;
};
