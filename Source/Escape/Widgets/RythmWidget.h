// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Arrow_Widget.h" // Include definition for TSubclassOf and TArray
#include "RythmWidget.generated.h"

// Forward declarations for bound widgets
class UVerticalBox;
class UImage;
class UActorComponent; // For StretchingComponent reference type
class UStretchingComponent; // Forward declare specific type

/**
 *  Enum representing the possible stretching directions required by the rhythm game lanes.
 * Used to map lanes to player input/state and check for correct poses during hits.
 */
UENUM(BlueprintType)
enum class EStretchState : uint8
{
    /** Stretch to the left (corresponds to Lane 0). */
    StretchLeft UMETA(DisplayName = "Stretch Left"),
    /** Stretch to the right (corresponds to Lane 1). */
    StretchRight UMETA(DisplayName = "Stretch Right"),
    /** Stretch upwards (corresponds to Lane 2). */
    StretchUp UMETA(DisplayName = "Stretch Up"),
    /** Stretch downwards (corresponds to Lane 3). */
    StretchDown UMETA(DisplayName = "Stretch Down")
};

/**
 *  URythmWidget
 * Widget class for the Rhythm mini-game UI, part of the stretching activity.
 * Handles spawning arrow widgets (UArrow_Widget), managing their movement down four lanes (VerticalBoxes),
 * detecting player hits based on timing and matching the required stretch pose (EStretchState)
 * obtained from the associated UStretchingComponent, and calculating the score.
 *
 * The lifecycle (spawning, updating, visibility) is controlled by external calls to StartRhythmGame() and StopRhythmGame().
 * The widget itself should be added/removed from the viewport by the owning component (e.g., UStretchingComponent).
 */
UCLASS()
class ESCAPE_API URythmWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     *  Called when the widget is constructed by the engine.
     * Initializes lane state mapping and calculates canvas height based on target zone positions.
     */
    virtual void NativeConstruct() override;

    /**
     *  Called every frame. If the game is active (bIsGameActive), updates the spawn timer,
     * spawns new arrows if necessary, and updates the positions of existing arrows, checking for hits/misses.
     *  MyGeometry The geometry of the widget.
     *  InDeltaTime The time elapsed since the last frame.
     */
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /**
     *  Sets the reference to the UStretchingComponent that controls the character's stretching actions.
     * This allows the widget to check the player's current stretch state when an arrow is hit.
     *  StretchhRef Pointer to the UStretchingComponent instance on the owning character.
     */
    UFUNCTION(BlueprintCallable, Category = "Rhythm|Setup")
    void SetStretchingComponent(UActorComponent* StretchhRef) { StretchingComponent = StretchhRef; };

    /**
     *  Starts the rhythm game logic.
     * Resets score and timers, clears any existing arrows, and enables ticking logic for spawning/updating arrows.
     * Does NOT add the widget to the viewport; that should be handled externally.
     */
    UFUNCTION(BlueprintCallable, Category = "Rhythm|Control")
    void StartRhythmGame();

    /**
     *  Stops the rhythm game logic.
     * Disables ticking logic, destroys all currently active arrows.
     * Does NOT remove the widget from the viewport; that should be handled externally.
     */
    UFUNCTION(BlueprintCallable, Category = "Rhythm|Control")
    void StopRhythmGame();

    // --- Configurable Properties ---

    /**
     *  The UUserWidget class to use for spawning arrows. Must be a subclass of UArrow_Widget.
     * Assign the corresponding Blueprint asset (e.g., WBP_Arrow_Widget) in the editor where this RythmWidget is used or created.
     */
    UPROPERTY(EditAnywhere, Category = "Rhythm|Setup")
    TSubclassOf<UArrow_Widget> ArrowWidgetClass;

    /**  The speed at which arrows travel down the lanes (in Unreal Units/pixels per second). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Gameplay", meta = (ClampMin = "1.0", UIMin = "1.0"))
    float ArrowSpeed = 300.0f;

    /**  The time interval (in seconds) between spawning new arrows. A lower value means arrows spawn more frequently. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Gameplay", meta = (ClampMin = "0.1", UIMin = "0.1"))
    float ArrowSpawnInterval = 1.0f;

    /**  The player's current score in the rhythm game. Updated on hits and misses. Reset by StartRhythmGame. */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rhythm|Gameplay")
    int32 Score = 0;

    /**  The vertical position (Y-coordinate) that defines the center of the hit timing window. Arrows must be hit near this line. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Gameplay")
    float HitZoneCenterY = 750.0f;

    /**  The tolerance (in Unreal Units/pixels) above and below the HitZoneCenterY that defines the hit window. A larger value makes timing easier. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Gameplay", meta = (ClampMin = "1.0", UIMin = "1.0"))
    float HitZoneTolerance = 27.0f;

    /**  Points awarded for hitting an arrow with the correct timing and pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Scoring")
    int32 ScorePerHit = 100;

    /**  Points deducted for hitting an arrow with the correct timing but the wrong pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Scoring")
    int32 PenaltyWrongPose = -50;

    /**  Points deducted for missing an arrow entirely (letting it go past the hit zone). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Scoring")
    int32 PenaltyMiss = -25;


protected:
    // --- Widget Bindings (Must be bound in the UMG Editor) ---

    /**  The vertical box representing the left lane where arrows travel. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> LeftLane;

    /**  The vertical box representing the right lane where arrows travel. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> RightLane;

    /**  The vertical box representing the up lane where arrows travel. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> UpLane;

    /**  The vertical box representing the down lane where arrows travel. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> DownLane;

    /**  The image representing the visual target zone indicator for the left lane. Used for calculating CanvasHeight. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> LeftTargetZone;

    /**  The image representing the visual target zone indicator for the right lane. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> RightTargetZone;

    /**  The image representing the visual target zone indicator for the up lane. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> UpTargetZone;

    /**  The image representing the visual target zone indicator for the down lane. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> DownTargetZone;

private:
    // --- Internal State ---

    /**  Flag indicating whether the rhythm game logic (spawning, updating) should execute in NativeTick. */
    bool bIsGameActive = false;

    /**  Array storing pointers to all currently active arrow widgets on screen. Managed by SpawnArrow and UpdateArrows. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UArrow_Widget>> ActiveArrows;

    /**  Array mapping the lane index (0-3) to the required EStretchState for that lane. Initialized in NativeConstruct. */
    TArray<EStretchState> LaneStates;

    /**  Timer used to track when to spawn the next arrow based on ArrowSpawnInterval. Counts down in NativeTick. */
    float SpawnTimer = 0.0f;

    /**  The estimated height of the playable area (lanes). Calculated in NativeConstruct based on target zone position. Used to detect missed arrows. */
    float CanvasHeight = 800.0f; // Default fallback value

    /**  Weak pointer reference to the UStretchingComponent. Used to check player state on hit. Set via SetStretchingComponent. */
    UPROPERTY(Transient)
    TWeakObjectPtr<UActorComponent> StretchingComponent;

    // --- Private Helper Methods ---

    /**  Spawns a new arrow widget (using ArrowWidgetClass) in a randomly selected lane and adds it to ActiveArrows. */
    void SpawnArrow();

    /**
     *  Updates the position of all arrows in ActiveArrows based on ArrowSpeed and DeltaTime.
     * Calls CheckHit to determine if an arrow is in the hit zone. Updates score based on hit/miss and player pose.
     * Removes arrows that are hit or missed.
     *  DeltaTime The time elapsed since the last frame.
     */
    void UpdateArrows(float DeltaTime);

    /**
     *  Checks if an arrow at a given Y position is within the designated hit timing window (HitZoneCenterY +/- HitZoneTolerance).
     *  Arrow The arrow widget being checked (currently unused in function body).
     *  LaneIndex The index of the lane the arrow is in (currently unused in function body).
     *  ArrowY The current Y position of the arrow (typically the bottom or center).
     * @return True if the arrow's Y position is within the hit window, false otherwise.
     */
    bool CheckHit(UUserWidget* Arrow, int32 LaneIndex, float ArrowY);

    /**
     *  Calculates the initial X position for spawning an arrow in a specific lane based on the layout of the lane widgets.
     *  LaneIndex The index of the lane (0=Left, 1=Right, 2=Up, 3=Down).
     * @return FVector2D containing the calculated X position and a default Y position (usually 0 or near the top).
     */
    FVector2D GetLanePosition(int32 LaneIndex);

    /**  Removes all arrows from the lanes and clears the ActiveArrows array. */
    void ClearActiveArrows();
};
