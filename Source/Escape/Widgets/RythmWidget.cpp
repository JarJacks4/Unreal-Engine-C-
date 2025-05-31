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
    // Ensure necessary components and the arrow widget class are valid before proceeding.
    if (!ArrowWidgetClass || !TargetZone)
    {
        return;
    }

    const int32 Position = FMath::RandRange(0, 3);

    UArrow_Widget* NewArrow = CreateWidget<UArrow_Widget>(GetOwningPlayer(), ArrowWidgetClass);    if (!NewArrow)
    {
        return;
    }

    // Determine the target lane and apply initial transformations based on the lane index.
    switch (Position)
    {
    case 0: // Left arrow
        NewArrow->SetRenderTransformAngle(180);
        break;
    case 1: // Right arrow
        break;
    case 2: // Up arrolw
        NewArrow->SetRenderTransformAngle(-90);
        break;
    case 3: // Down arrow
        NewArrow->SetRenderTransformAngle(90);
        break;
    default:
        NewArrow->RemoveFromParent(); // Clean up
        return;
    }
}

/**
 *  Updates the position of all active arrows, checks for hits/misses based on timing and player pose, updates score, and removes processed arrows.
 *  DeltaTime The time elapsed since the last frame.
 */
void URythmWidget::UpdateArrows(float DeltaTime)
{
    // Get the player's current stretch state once for efficiency.
    EStretchState PlayerCurrentState = EStretchState::StretchLeft; // Default if component invalid
    if (!CachedStretchingComponent)
    {
        CachedStretchingComponent = StretchingComponent.IsValid() ? Cast<UStretchingComponent>(StretchingComponent.Get()) : nullptr;
    }
    UStretchingComponent* StretchComp = CachedStretchingComponent;
    if (StretchComp)
    {
        PlayerCurrentState = StretchComp->GetCurrentStretchState();
    }

}