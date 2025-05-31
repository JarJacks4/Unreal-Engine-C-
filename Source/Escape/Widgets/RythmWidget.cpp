// Fill out your copyright notice in the Description page of Project Settings.

#include "RythmWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h" // Required for casting Slot to get position/size
#include "Kismet/GameplayStatics.h" // Potentially needed for sound effects
#include "Kismet/KismetSystemLibrary.h" // Potentially needed for debug drawing
#include "../Components/StretchingComponent.h" // Include specific component for GetCurrentStretchState
#include "Arrow_Widget.h" // Ensure Arrow_Widget definition is included

static UStretchingComponent* CachedStretchingComponent = nullptr;

/**
 *  Called when the widget is constructed by the engine.
 * Initializes the mapping of lane indices to required stretch states.
 * Calculates the effective canvas height based on the position of the target zones,
 * which is used to determine when an arrow is missed.
 */
void URythmWidget::NativeConstruct()
{
    Super::NativeConstruct();


    // Initialize the spawn timer. StartRhythmGame will reset it.
    SpawnTimer = 0.0f;
    bIsGameActive = false; // Ensure game is not active initially

}

/**
 *  Called every frame. If the game is active (bIsGameActive), updates the spawn timer, spawns new arrows if necessary, and updates existing arrow positions, handling hits and misses.
 *  MyGeometry The geometry of the widget.
 *  InDeltaTime The time elapsed since the last frame.
 */
void URythmWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Only run game logic if the game is active
    if (!bIsGameActive)
    {
        return;
    }

    // --- Arrow Spawning ---
    SpawnTimer -= InDeltaTime;
    if (SpawnTimer <= 0.0f)
    {
        SpawnArrow();
        SpawnTimer = ArrowSpawnInterval; // Reset timer for the next spawn.
    }

    // --- Arrow Updating ---
    UpdateArrows(InDeltaTime);
}

/**
 *  Starts the rhythm game logic.
 * Resets score and timers, clears any existing arrows, and enables ticking logic for spawning/updating arrows.
 */
void URythmWidget::StartRhythmGame()
{
    ClearActiveArrows(); // Clear any remnants from a previous session
    Score = 0;
    SpawnTimer = 0.0f; // Spawn first arrow immediately
    bIsGameActive = true;
    // Note: Visibility and adding to viewport are handled by the caller (StretchingComponent)
}

/**
 *  Stops the rhythm game logic.
 * Disables ticking logic and destroys all currently active arrows.
 */
void URythmWidget::StopRhythmGame()
{
    bIsGameActive = false;
    ClearActiveArrows();
    // Note: Visibility and removing from viewport are handled by the caller (StretchingComponent)
}

/**
 *  Removes all arrows from the lanes and clears the ActiveArrows array.
 */
void URythmWidget::ClearActiveArrows()
{
    // Iterate backwards to safely remove elements
    for (int32 i = ActiveArrows.Num() - 1; i >= 0; --i)
    {
        if (ActiveArrows[i])
        {
            ActiveArrows[i]->RemoveFromParent(); // Remove widget from hierarchy
        }
    }
    ActiveArrows.Empty(); // Clear the array
}


/**
 *  Spawns a new arrow widget (using ArrowWidgetClass) in a randomly selected lane.
 * Adds the arrow to the appropriate lane's VerticalBox and the ActiveArrows array.
 * Applies initial rotation/scale based on the lane.
 */
void URythmWidget::SpawnArrow()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Spawning Arrow 1..."));
    // Ensure necessary components and the arrow widget class are valid before proceeding.
    if (!ArrowWidgetClass || !TargetZone)
    {
        return;
    }
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Spawning Arrow 2..."));
    const int8 Position = FMath::RandRange(0,3);
    
    UArrow_Widget* NewArrow = CreateWidget<UArrow_Widget>(GetOwningPlayer(), ArrowWidgetClass);    if (!NewArrow)
    {
        return;
    }
    SpawnZonesContainer->AddChild(NewArrow); // Add the new arrow widget to the canvas panel
	Cast<UCanvasPanelSlot>(NewArrow->Slot)->SetSize(FVector2D(256, 256)); // Set size of the arrow widget
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Spawning Arrow at Position: %d"), Position));
    switch (Position)
    {
    case 0: // Left arrow
		NewArrow->StretchState = EStretchState::StretchLeft;
		Cast<UCanvasPanelSlot>(NewArrow->Slot)->SetPosition(Cast<UCanvasPanelSlot>(SpawnZones[0]->Slot)->GetPosition()); // Set initial position
        NewArrow->SetRenderTransformAngle(180);
        break;
    case 1: // Right arrow
		NewArrow->StretchState = EStretchState::StretchRight;
        Cast<UCanvasPanelSlot>(NewArrow->Slot)->SetPosition(Cast<UCanvasPanelSlot>(SpawnZones[1]->Slot)->GetPosition()); // Set initial position
        NewArrow->SetRenderTransformAngle(-180);

        break;
    case 2: // Up arrow
		NewArrow->StretchState = EStretchState::StretchUp;
        Cast<UCanvasPanelSlot>(NewArrow->Slot)->SetPosition(Cast<UCanvasPanelSlot>(SpawnZones[2]->Slot)->GetPosition()); // Set initial position

        NewArrow->SetRenderTransformAngle(-90);
        break;
    case 3: // Down arrow
		NewArrow->StretchState = EStretchState::StretchDown;
        Cast<UCanvasPanelSlot>(NewArrow->Slot)->SetPosition(Cast<UCanvasPanelSlot>(SpawnZones[3]->Slot)->GetPosition()); // Set initial position
        NewArrow->SetRenderTransformAngle(90);
        break;
    default:
        NewArrow->RemoveFromParent(); // Clean up
        return;
    }
	ActiveArrows.Add(NewArrow); // Add to active arrows list
}

/**
 *  Updates the position of all active arrows, checks for hits/misses based on timing and player pose, updates score, and removes processed arrows.
 *  DeltaTime The time elapsed since the last frame.
 */
void URythmWidget::UpdateArrows(float DeltaTime)
{
    
    for (size_t i = 0; i < ActiveArrows.Num(); i++)
    {
        UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(ActiveArrows[i]->Slot);
        ArrowSlot->SetPosition( FVector2D(ArrowSlot->GetPosition().X, ArrowSlot->GetPosition().Y + DeltaTime * ArrowSpeed));

        if (ArrowSlot->GetPosition().Y >= Cast<UCanvasPanelSlot>(TargetZone->Slot)->GetPosition().Y + HitZoneTolerance) {
            ActiveArrows[i]->RemoveFromParent(); // Remove missed arrow
			ActiveArrows.RemoveAt(i); // Remove from active arrows list
            

            Score -= PenaltyMiss;
			if (Score <= 0) {
				Score = 0; // Ensure score doesn't go negative
			}
        }
    }
}