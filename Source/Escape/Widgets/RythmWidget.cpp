// Fill out your copyright notice in the Description page of Project Settings.

#include "RythmWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h" // Required for casting Slot to get position/size
#include "Kismet/GameplayStatics.h" // Potentially needed for sound effects
#include "Kismet/KismetSystemLibrary.h" // Potentially needed for debug drawing
#include "../Components/StretchingComponent.h" // Include specific component for GetCurrentStretchState
#include "Arrow_Widget.h" // Ensure Arrow_Widget definition is included

static UCanvasPanelSlot* CachedLeftTargetZoneSlot = nullptr;
static UCanvasPanelSlot* CachedTargetLaneWidgetSlot = nullptr;
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

    // Initialize the mapping between lane index and the required stretch state.
    LaneStates.Init(EStretchState::StretchLeft, 4); // Initialize with a default value, size 4
    LaneStates[0] = EStretchState::StretchLeft;    // Lane 0 corresponds to StretchLeft
    LaneStates[1] = EStretchState::StretchRight;   // Lane 1 corresponds to StretchRight
    LaneStates[2] = EStretchState::StretchUp;      // Lane 2 corresponds to StretchUp
    LaneStates[3] = EStretchState::StretchDown;    // Lane 3 corresponds to StretchDown

    // Initialize the spawn timer. StartRhythmGame will reset it.
    SpawnTimer = 0.0f;
    bIsGameActive = false; // Ensure game is not active initially

    // Attempt to calculate the effective canvas height based on the position of the LeftTargetZone.
    // This assumes the target zones are positioned at the bottom where arrows should be hit or missed.
    if (LeftTargetZone)
    {
        if (!CachedLeftTargetZoneSlot)
        {
            CachedLeftTargetZoneSlot = Cast<UCanvasPanelSlot>(LeftTargetZone->Slot);
        }

        if (CachedLeftTargetZoneSlot)
        {
            CanvasHeight = HitZoneCenterY + HitZoneTolerance + 10.0f; // Point slightly past the hit zone
        }
    }
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
    if (!ArrowWidgetClass || !LeftLane || !RightLane || !UpLane || !DownLane)
    {
        return;
    }

    const int32 LaneIndex = FMath::RandRange(0, 3);
    UVerticalBox* TargetLane = nullptr;

    UArrow_Widget* NewArrow = CreateWidget<UArrow_Widget>(GetOwningPlayer(), ArrowWidgetClass);    if (!NewArrow)
    {
        return;
    }
    NewArrow->ConceptualYPosition = 0.0f; 

    // Determine the target lane and apply initial transformations based on the lane index.
    switch (LaneIndex)
    {
    case 0: // Left Lane
        TargetLane = LeftLane;
        NewArrow->SetRenderTransformAngle(180);
        break;
    case 1: // Right Lane
        TargetLane = RightLane;
        break;
    case 2: // Up Lane
        TargetLane = UpLane;
        NewArrow->SetRenderTransformAngle(-90);
        break;
    case 3: // Down Lane
        TargetLane = DownLane;
        NewArrow->SetRenderTransformAngle(90);
        break;
    default:
        NewArrow->RemoveFromParent(); // Clean up
        return;
    }

    if (TargetLane)
    {
        TargetLane->AddChildToVerticalBox(NewArrow);
        ActiveArrows.Add(NewArrow);
    }
    else
    {
        NewArrow->RemoveFromParent(); // Clean up
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

    // Iterate backwards through the list of active arrows to allow safe removal during iteration.
    for (int32 i = ActiveArrows.Num() - 1; i >= 0; --i)
    {
        UArrow_Widget* Arrow = ActiveArrows[i];
        if (!Arrow) { /* ... remove null arrow ... */ continue; }

        // --- Arrow Movement (Conceptual) ---
        // Update the conceptual position stored on the arrow widget itself
        Arrow->ConceptualYPosition += (ArrowSpeed * DeltaTime); // THIS LINE UPDATES THE POSITION

        // --- Hit/Miss Detection ---
        int32 LaneIndex = -1;
        // ... (Determine LaneIndex) ...

        bool bArrowProcessed = false;

        if (LaneIndex >= 0)
        {
            // Use the updated ConceptualYPosition from the Arrow object for hit check
            if (CheckHit(Arrow, LaneIndex, Arrow->ConceptualYPosition)) // USE Arrow->ConceptualYPosition
            {
                // ... (Handle hit: score, remove arrow) ...
                bArrowProcessed = true;
            }
        }
        else
        {
            // ... (Handle arrow not in lane: remove arrow) ...
            bArrowProcessed = true;
        }

        // --- Miss Detection ---
        // Use the updated ConceptualYPosition from the Arrow object for miss check
        if (!bArrowProcessed && Arrow->ConceptualYPosition > CanvasHeight) // USE Arrow->ConceptualYPosition
        {
            // ... (Handle miss: score, remove arrow) ...
        }
    }
}

/**
 *  Checks if an arrow's conceptual Y position falls within the hit timing window.
 *  Arrow The arrow widget (unused).
 *  LaneIndex The lane index (unused).
 *  ArrowY The conceptual Y position of the arrow.
 * @return True if ArrowY is within [HitZoneCenterY - HitZoneTolerance, HitZoneCenterY + HitZoneTolerance].
 */
bool URythmWidget::CheckHit(UUserWidget* Arrow, int32 LaneIndex, float ArrowY)
{
    const float HitZoneStartY = HitZoneCenterY - HitZoneTolerance;
    const float HitZoneEndY = HitZoneCenterY + HitZoneTolerance;

    // Check if the arrow's Y position falls within the hit zone boundaries.
    return (ArrowY >= HitZoneStartY && ArrowY <= HitZoneEndY);
}

/**
 *  Calculates the initial X position for spawning an arrow in a specific lane.
 * Tries to get the position from the lane widget's slot if it's in a CanvasPanel.
 * Falls back to a placeholder calculation if position cannot be determined.
 *  LaneIndex The index of the lane (0=Left, 1=Right, 2=Up, 3=Down).
 * @return FVector2D containing the calculated X position and a starting Y position (0.0f).
 */
FVector2D URythmWidget::GetLanePosition(int32 LaneIndex)
{
    float TargetX = 0.0f;
    UWidget* TargetLaneWidget = nullptr;

    switch (LaneIndex)
    {
        case 0: TargetLaneWidget = LeftLane; break;
        case 1: TargetLaneWidget = RightLane; break;
        case 2: TargetLaneWidget = UpLane; break;
        case 3: TargetLaneWidget = DownLane; break;
        default:
            return FVector2D(0.0f, 0.0f);
    }

    if (TargetLaneWidget)
    {
        if (!CachedTargetLaneWidgetSlot)
        {
            CachedTargetLaneWidgetSlot = Cast<UCanvasPanelSlot>(TargetLaneWidget->Slot);
        }

        if (CachedTargetLaneWidgetSlot)
        {
            TargetX = CachedTargetLaneWidgetSlot->GetPosition().X;
        }
        else
        {
            const float LaneWidthEstimate = 150.0f;
            const float StartXOffset = 200.0f;
            TargetX = StartXOffset + (LaneIndex * LaneWidthEstimate);
        }
    }
    else
    {
        const float LaneWidthEstimate = 150.0f;
        const float StartXOffset = 200.0f;
        TargetX = StartXOffset + (LaneIndex * LaneWidthEstimate);
    }

    return FVector2D(TargetX, 0.0f);
}
