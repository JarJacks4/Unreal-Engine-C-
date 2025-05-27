// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" // Use CoreMinimal for basic types if possible
#include "Blueprint/UserWidget.h"
#include "Arrow_Widget.generated.h"

/**
 *  UArrow_Widget
 * Represents a single visual arrow element used in the rhythm-based stretching game (URythmWidget).
 * This widget is spawned dynamically by URythmWidget, placed into one of the lanes (VerticalBoxes),
 * and moves downwards. Its appearance (e.g., direction indicator) might be set based on the lane it's spawned in.
 *
 * This class currently serves as a base class, likely intended to be subclassed in Blueprint (WBP_Arrow_Widget)
 * where the actual visual elements (like an Image component) are added.
 */
UCLASS()
class ESCAPE_API UArrow_Widget : public UUserWidget
{
    GENERATED_BODY()
public:
    float ConceptualYPosition = 0.0f;

};
