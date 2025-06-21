#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h" // Include necessary component headers
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "InputActionValue.h" // Include if input actions are directly handled here (currently not)
#include "WellnessBlock.generated.h"

// Forward declarations
class AEscapeCharacter;
class UMobileUIWidget; // Forward declare for UI interaction

/**
 *  Enum defining the type of wellness activity associated with a WellnessBlock.
 * This determines the behavior triggered when the player interacts with the block.
 */
UENUM(BlueprintType)
enum class EWellnessBlockType : uint8
{
    /**  The block triggers the stretching activity (rhythm game). */
    Stretching UMETA(DisplayName = "Stretching"),
    /**  The block triggers the meditation activity (balancing/timing). */
    Meditation UMETA(DisplayName = "Meditation"),
    /**  The block triggers the deep breathing activity (bubble timing). */
    DeepBreathing UMETA(DisplayName = "Deep Breathing"),
    /**  The block triggers the journaling activity. */
    Journaling UMETA(DisplayName = "Journaling"), // Added Journaling type
    /**  Represents an unassigned or default state. Interaction might default to Deep Breathing or do nothing. */
    None UMETA(DisplayName = "None")
};

/**
 *  Enum defining the different states of the meditation block's visual levitation effect.
 * Controls the vertical movement animation of the block during meditation interaction.
 */
UENUM(BlueprintType)
enum class EMeditationBlockState : uint8
{
    /**  The block is at its resting position, not moving vertically. */
    None UMETA(DisplayName = "None"),
    /**  The block is moving upwards from its resting position towards the maximum height. */
    Rising UMETA(DisplayName = "Rising"),
    /**  The block is gently floating upwards between the lowest and maximum float heights. */
    FloatingUp UMETA(DisplayName = "Floating Up"),
    /**  The block is gently floating downwards between the maximum and lowest float heights. */
    FloatingDown UMETA(DisplayName = "Floating Down"),
    /**  The block is moving downwards from its elevated position back to the resting position. */
    Lowering UMETA(DisplayName = "Lowering")
};

/**
 *  AWellnessBlock
 * An interactive Actor placed in the game world that serves as an entry point for player wellness activities.
 * It detects player proximity using a trigger volume, communicates the activity type to the player character,
 * and manages visual effects like rotation and levitation (specifically for meditation blocks).
 */
UCLASS()
class ESCAPE_API AWellnessBlock : public AActor
{
    GENERATED_BODY()
public:
    /**  Default constructor. Initializes components, sets default property values, and binds overlap events. */
    AWellnessBlock();

    // --- Configuration: Visuals ---
    /**
	 *  The location at which the block mesh will be placed in the world.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    FVector InitialLocation;

    /**
     *  The speed at which the block mesh rotates back and forth around its Z-axis (Roll) when idle or not overridden by player input.
     * Units: Degrees per second. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float RotationSpeed = 100.0f;

    /**  Sets the current state of the meditation block. This is used to determine the block's levitation and rotation behavior. */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationBlockState(EMeditationBlockState NewMeditationBlockState) { MeditationBlockState = NewMeditationBlockState; };
	/**  Gets the current state of the meditation block. This is used to determine the block's levitation and rotation behavior. */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    EMeditationBlockState GetMeditationBlockState() { return MeditationBlockState; };

    /**
     *  The maximum angle (in degrees) the block will rotate from its central orientation during the idle oscillation or in response to player tilt input.
     * Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MaxRotationAngle = 45.0f;

    /**
     *  The maximum vertical distance the block will rise from its initial Z position when the meditation activity starts.
     * Units: Unreal Units. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visuals|Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MaxDistanceFromGround = 100.0f;

    /**
     *  The lowest vertical position the block reaches relative to its initial Z position while performing the floating animation during meditation.
     * Can be 0 or slightly negative/positive. Units: Unreal Units. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation", meta = (ClampMin = "-50.0", UIMin = "-50.0")) // Allow slight negative float
    float LowestDistanceFromGround = 0.0f;

    /**
     *  The speed at which the block floats up and down between LowestDistanceFromGround and MaxDistanceFromGround during the FloatingUp/FloatingDown states.
     * Units: Unreal Units per second. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MovementSpeed = 20.0f;

    /**
     *  The speed at which the block moves downwards back to its resting position during the Lowering state (when meditation ends or player leaves).
     * Units: Unreal Units per second. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float LowerSpeed = 50.0f;

    /**
     *  The speed at which the block moves upwards from its resting position during the Rising state (when meditation begins).
     * Units: Unreal Units per second. Adjustable in the editor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float RiseSpeed = 50.0f;

    // --- Configuration: Gameplay ---

    /**
     *  The type of wellness activity this block initiates (e.g., Stretching, Meditation, Deep Breathing).
     * Determines the behavior when the player interacts. Set this in the editor for each block instance.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness")
    EWellnessBlockType BlockType = EWellnessBlockType::None;

    // --- Components ---

    /**
     *  The root component for this Actor. A CapsuleComponent is used here, providing a basic shape, though a simple USceneComponent might suffice if no root collision is needed.
     */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> SceneRootComponent;

    /**  The Static Mesh Component that defines the visual appearance of the block. Attach your block mesh asset here in the editor. */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BlockMesh;

    /**  The Box Collision Component used as a trigger volume to detect when the player enters or leaves the interaction range. */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> TriggerVolume;

    // --- Runtime State & Accessors ---

    /**  Gets the current rotation angle (Roll) of the block mesh relative to its starting orientation. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Visuals|Rotation") // Added BlueprintPure
    float GetCurrentRotation() const { return CurrentRotation; }

    /**  A weak pointer reference to the player character currently overlapping/interacting with this block. Null if no player is interacting. */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction", Transient) // Transient, state is runtime only
    TWeakObjectPtr<ACharacter> PlayerRef;

    /**
     *  Called every frame. Updates the block's rotation and levitation based on its state, player interaction, and time.
     *  DeltaTime Game time elapsed during the last frame.
     */
    virtual void Tick(float DeltaTime) override;

protected:
    /**
     *  Called when the game starts or when the actor is spawned.
     * Performs initial setup, such as validating speed properties.
     */
    virtual void BeginPlay() override;

    /**
     *  Callback function executed when an actor begins overlapping with the TriggerVolume.
     * Checks if the overlapping actor is the player, stores a reference, updates player state, and potentially triggers UI changes or block animations (like Rising).
     *  OverlappedComp The component that was overlapped (the TriggerVolume).
     *  OtherActor The actor that entered the overlap volume.
     *  OtherComp The specific component of the OtherActor that caused the overlap.
     *  OtherBodyIndex Index of the physics body if applicable.
     *  bFromSweep Whether the overlap resulted from a movement sweep query.
     *  SweepResult Hit result information if the overlap was from a sweep.
     */
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
     *  Callback function executed when an actor stops overlapping with the TriggerVolume.
     * Checks if the exiting actor was the interacting player, clears the reference, resets player state, updates UI, and potentially triggers block animations (like Lowering).
     *  OverlappedComp The component that is no longer overlapped (the TriggerVolume).
     *  OtherActor The actor that exited the overlap volume.
     *  OtherComp The specific component of the OtherActor that ended the overlap.
     *  OtherBodyIndex Index of the physics body if applicable.
     */
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // --- Internal State: Rotation ---
    /**  Internal variable tracking the current roll rotation relative to the initial orientation. */
    float CurrentRotation = 0.0f;
    /**  Internal variable storing the target rotation angle, used for interpolation based on player tilt input. */
    float TargetRotation = 0.0f;
    /**  Internal flag indicating the current direction of the default oscillation rotation (true for right/positive, false for left/negative). */
    bool bIsRotatingRight = true;

    // --- Internal State: Levitation ---
    /**  Internal variable storing the current state of the meditation levitation effect (None, Rising, FloatingUp, etc.). */
    EMeditationBlockState MeditationBlockState = EMeditationBlockState::None;

    /**  Cached reference to the player as AEscapeCharacter for efficient cast reuse. */
    AEscapeCharacter* CachedPlayerCharacter = nullptr;

    // --- Internal Methods ---
    /**
     *  Updates the CurrentRotation based on RotationSpeed, player tilt (if meditating), or default oscillation. Called within Tick.
     *  DeltaTime Game time elapsed during the last frame.
     */
    void UpdateRotation(float DeltaTime);

    /**  Applies the calculated CurrentRotation to the BlockMesh component's relative rotation. Called by UpdateRotation. */
    void ApplyRotation();

    /**
     *  Updates the vertical position (Z-location) of the BlockMesh based on the current MeditationBlockState and associated speeds. Called within Tick for Meditation blocks.
     *  DeltaTime Game time elapsed during the last frame.
     */
    void UpdateLevitation(float DeltaTime);
};
