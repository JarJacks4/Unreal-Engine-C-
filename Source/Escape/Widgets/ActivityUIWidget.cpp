#include "ActivityUIWidget.h"


void UActivityUIWidget::NativeConstruct() {
	Super::NativeConstruct();
	// Initialize the RythmWidget if the class is set
	if (RythmWidgetClass)
	{
		RythmWidget = CreateWidget<URythmWidget>(GetWorld(), RythmWidgetClass);
		if (RythmWidget)
		{
			RythmWidget->AddToViewport(1); // Add to viewport with a specific Z-order
			RythmWidget->SetVisibility(ESlateVisibility::Hidden); // Start hidden
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UActivityUIWidget::NativeConstruct - Failed to create RythmWidget instance."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UActivityUIWidget::NativeConstruct - RythmWidgetClass is not set. Rythm widget will not be created."));
	}
}