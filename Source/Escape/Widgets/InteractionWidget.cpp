// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Styling/ISlateStyle.h"

void UInteractionWidget::SetWidgetMeditationImage() {
	if (MeditationImage) {
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(MeditationImage);
		NewBrush.ImageSize = FVector2D(200, 200); // Adjust size as needed
		NewBrush.DrawAs = ESlateBrushDrawType::Image;

		FButtonStyle ButtonStyle = InteractionWidgetButton->GetStyle();
		ButtonStyle.SetNormal(NewBrush);
		ButtonStyle.SetHovered(NewBrush); 
		ButtonStyle.SetPressed(NewBrush);
		ButtonStyle.SetDisabled(NewBrush);
		InteractionWidgetButton->SetStyle(ButtonStyle);
	}
}

void UInteractionWidget::SetWidgetStretchingImage() {
	if (StretchingImage) {
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(StretchingImage);
		NewBrush.ImageSize = FVector2D(200, 200); // Adjust size as needed
		NewBrush.DrawAs = ESlateBrushDrawType::Image;

		FButtonStyle ButtonStyle = InteractionWidgetButton->GetStyle();
		ButtonStyle.SetNormal(NewBrush);
		ButtonStyle.SetHovered(NewBrush); 
		ButtonStyle.SetPressed(NewBrush);
		ButtonStyle.SetDisabled(NewBrush);
		InteractionWidgetButton->SetStyle(ButtonStyle);
	}
}

void UInteractionWidget::SetWidgetDeepBreathingImage() {
	if (DeepBreathingImage) {
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(DeepBreathingImage);
		NewBrush.ImageSize = FVector2D(200, 200); // Adjust size as needed
		NewBrush.DrawAs = ESlateBrushDrawType::Image;

		FButtonStyle ButtonStyle = InteractionWidgetButton->GetStyle();
		ButtonStyle.SetNormal(NewBrush);
		ButtonStyle.SetHovered(NewBrush); 
		ButtonStyle.SetPressed(NewBrush);
		ButtonStyle.SetDisabled(NewBrush);
		InteractionWidgetButton->SetStyle(ButtonStyle);
	}
}