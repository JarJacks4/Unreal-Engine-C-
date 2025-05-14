#include "WellnessBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h" // Include header for root component type
#include "Components/MeditationComponent.h" // Although not directly used, related to meditation logic
#include "InputCoreTypes.h" // Included but seemingly unused
#include "Input/Events.h" // Included but seemingly unused
#include "Components/WidgetComponent.h" // Included but seemingly unused
#include "Engine/GameViewportClient.h" // Included but seemingly unused
#include "GenericPlatform/GenericPlatformMisc.h" // Included but seemingly unused
#include "EscapeCharacter.h" // Include player character header for interaction logic
#include "Widgets/MobileUIWidget.h" // Include mobile UI header for interaction prompts
#include "Kismet/KismetMathLibrary.h" // For FInterpTo
#include "Components/WellnessComponent.h" // Include wellness component header for interaction logic


/**
 *  Constructor for the AWellnessBlock class.
 * Initializes components (Capsule Root, StaticMesh, Box Trigger), sets up attachments,
 * binds overlap events, enables ticking, and initializes default values for rotation and levitation state.
 */
AWellnessBlock::AWellnessBlock()
{
    // Enable the Tick function to be called every frame for updating rotation and levitation.
    PrimaryActorTick.bCanEverTick = true;

    // Create the root component using a CapsuleComponent.
    SceneRootComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SceneRootComponent"));
    RootComponent = SceneRootComponent; // Set it as the actor's root component.
    // Initialize capsule size (adjust if the capsule needs specific dimensions for collision/queries).
    SceneRootComponent->InitCapsuleSize(1.f, 2.0f);
    // Set collision profile (example: NoCollision if only used as a root anchor, or a custom profile if needed).
    SceneRootComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

    // Create the static mesh component for the block's visual representation.
    BlockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlockMesh"));
    BlockMesh->SetupAttachment(RootComponent); // Attach the mesh to the root component.
    // Set collision profile for the mesh (example: BlockAllDynamic or a custom profile).
    BlockMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

    // Create the box component used as a trigger volume for player interaction.
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(BlockMesh); // Attach the trigger volume to the mesh component.
    // Set collision profile to trigger overlaps with Pawns.
    TriggerVolume->SetCollisionProfileName(FName(TEXT("Trigger")));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Only needs to detect overlaps, not block physics.
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore); // Ignore all channels by default...
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // ...then specifically enable overlap for Pawns.
    // Set default size (adjust in editor or here as needed).
    TriggerVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));

    // Bind the overlap event functions to the component's delegates.
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWellnessBlock::OnOverlapBegin);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AWellnessBlock::OnOverlapEnd);

    // Initialize internal state variables.
    CurrentRotation = 0.0f;      // Start with no relative rotation.
    TargetRotation = 0.0f;       // Initial target rotation for interpolation.
    bIsRotatingRight = true;     // Start default oscillation rotating to the right.
    MeditationBlockState = EMeditationBlockState::None; // Initial meditation levitation state.
    CachedPlayerCharacter = nullptr; // Initialize cached player character.
}

/**
 *  Called when the game starts or when the actor is spawned.
 * Validates configurable speed properties to prevent issues like division by zero or unintended behavior.
 */
void AWellnessBlock::BeginPlay()
{
    Super::BeginPlay(); // Call the parent class's BeginPlay.
	InitialLocation = BlockMesh->GetRelativeLocation(); // Store the initial location of the block.
    // Sanitize speed values to ensure they are positive. Log warnings if adjustments are made.
    if (LowerSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WellnessBlock '%s': LowerSpeed is non-positive (%.2f), setting to 1.0."), *GetName(), LowerSpeed);
        LowerSpeed = 1.0f;
    }
    if (RiseSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WellnessBlock '%s': RiseSpeed is non-positive (%.2f), setting to 1.0."), *GetName(), RiseSpeed);
        RiseSpeed = 1.0f;
    }
    if (MovementSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WellnessBlock '%s': MovementSpeed (for floating) is non-positive (%.2f), setting to 1.0."), *GetName(), MovementSpeed);
        MovementSpeed = 1.0f;
    }
    if (RotationSpeed < 0.0f) // Rotation speed can be 0, but not negative
    {
        UE_LOG(LogTemp, Warning, TEXT("WellnessBlock '%s': RotationSpeed is negative (%.2f), setting to 0.0."), *GetName(), RotationSpeed);
        RotationSpeed = 0.0f;
    }
}

/**
 *  Called every frame. Updates the block's rotation and levitation (if applicable) based on its state and player interaction.
 *  DeltaTime The time elapsed since the last frame.
 */
void AWellnessBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // Call the parent class's Tick.

    

    // Update Levitation: Only perform levitation logic for Meditation blocks AND when the state is not 'None'.
    if (BlockType == EWellnessBlockType::Meditation && MeditationBlockState != EMeditationBlockState::None)
    {
        // Levitation logic also implicitly depends on PlayerRef being valid within UpdateLevitation.
        UpdateLevitation(DeltaTime);
        // Update Rotation: Perform the rotation logic (oscillation or tilt response).
        //UpdateRotation(DeltaTime);
    }
}


/**
 *  Updates the rotation of the block mesh.
 * If the interacting player is meditating, the rotation interpolates towards a target angle based on player tilt input.
 * Otherwise, it performs a default back-and-forth oscillation between -MaxRotationAngle and +MaxRotationAngle.
 *  DeltaTime The time elapsed since the last frame.
 */
void AWellnessBlock::UpdateRotation(float DeltaTime)
{
    // Ensure BlockMesh is valid before attempting to rotate it.
    if (!BlockMesh) return;

    // Default interpolation speed for returning to oscillation or handling tilt.
    const float InterpSpeed = 5.0f;

    // Check if a player is interacting and meditating.
    if (CachedPlayerCharacter && CachedPlayerCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::Meditating)
    {
        // Player is meditating: Control rotation based on tilt input.
        float TiltInput = CachedPlayerCharacter->GetTilt();
        // Calculate the target rotation based on the tilt input and max angle.
        TargetRotation = TiltInput * MaxRotationAngle;

        // Smoothly interpolate the current rotation towards the target rotation.
        CurrentRotation = FMath::FInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
    }
    else
    {
        // Player is not meditating (or no player interacting): Perform default oscillation.

        // Calculate the rotation change for this frame based on speed.
        const float RotationDelta = RotationSpeed * DeltaTime;

        // Determine the target for this frame based on oscillation direction.
        float OscillationTarget = bIsRotatingRight ? MaxRotationAngle : -MaxRotationAngle;

        // Move towards the oscillation target.
        if (bIsRotatingRight)
        {
            CurrentRotation += RotationDelta;
            // If the max angle is reached or exceeded, clamp and reverse direction.
            if (CurrentRotation >= MaxRotationAngle)
            {
                CurrentRotation = MaxRotationAngle;
                bIsRotatingRight = false;
            }
        }
        else // Rotating left
        {
            CurrentRotation -= RotationDelta;
            // If the min angle is reached or exceeded, clamp and reverse direction.
            if (CurrentRotation <= -MaxRotationAngle)
            {
                CurrentRotation = -MaxRotationAngle;
                bIsRotatingRight = true;
            }
        }
    }

    // Clamp the rotation to ensure it stays within the defined bounds, regardless of method.
    CurrentRotation = FMath::Clamp(CurrentRotation, -MaxRotationAngle, MaxRotationAngle);

    // Apply the final calculated rotation to the mesh.
    //ApplyRotation();
}

/**
 *  Applies the calculated CurrentRotation as a relative rotation (Roll) to the BlockMesh component.
 */
void AWellnessBlock::ApplyRotation()
{
    if (BlockMesh)
    {
        // Create a new rotator with the CurrentRotation applied to the Roll axis (Z-axis rotation).
        const FRotator NewRotation(0.0f, 0.0f, CurrentRotation);
        // Apply the rotation relative to the parent component (SceneRootComponent).
        BlockMesh->SetRelativeRotation(NewRotation);
    }
}

/**
 *  Handles the event when an actor begins overlapping the trigger volume.
 * Stores a reference to the player, updates the player's interacting block type,
 * updates the mobile UI prompt, and initiates the Rising state for Meditation blocks.
 *  OverlappedComp The component that was overlapped (TriggerVolume).
 *  OtherActor The actor that entered the volume.
 *  OtherComp The component of the other actor that overlapped.
 *  OtherBodyIndex Body index for physics simulation.
 *  bFromSweep True if the overlap was caused by a movement sweep.
 *  SweepResult Detailed information about the sweep hit.
 */
void AWellnessBlock::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a valid player character and not this block itself.
    if (!CachedPlayerCharacter && OtherActor) CachedPlayerCharacter = Cast<AEscapeCharacter>(OtherActor);
    if (CachedPlayerCharacter)
    {
        // Store a weak reference to the player character.
        
        // Notify the character that they are now inside a block's trigger.
        CachedPlayerCharacter->bIsInBlock = true;
        
        // Set the block type on the player character to match this block's type
        CachedPlayerCharacter->SetBlockType(BlockType);
        if (BlockType == EWellnessBlockType::Meditation)
        {
            CachedPlayerCharacter->GetMobileUIWidget()->InteractionWidget->SetWidgetMeditationImage(); // Update the mobile UI prompt to show meditation image
        }

        if(BlockType == EWellnessBlockType::Stretching)
        {
            CachedPlayerCharacter->GetMobileUIWidget()->InteractionWidget->SetWidgetStretchingImage(); // Update the mobile UI prompt to show stretching image
        }
        // Update the player character's meditation component with a reference to this block
        CachedPlayerCharacter->MeditationComponent->SetBlockRef(this);
        

    }
}

/**
 *  Handles the event when an actor stops overlapping the trigger volume.
 * Clears the player reference, resets the player's interacting block type, updates the mobile UI,
 * and initiates the Lowering state for Meditation blocks if the player leaves while it's elevated.
 *  OverlappedComp The component that is no longer overlapped (TriggerVolume).
 *  OtherActor The actor that exited the volume.
 *  OtherComp The component of the other actor that ended the overlap.
 *  OtherBodyIndex Body index for physics simulation.
 */
void AWellnessBlock::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Check if the actor leaving is the player character currently referenced by this block.
    if (CachedPlayerCharacter && CachedPlayerCharacter == Cast<AEscapeCharacter>(OtherActor))
    {
        // Notify the character that they have left the block's trigger.
        CachedPlayerCharacter->bIsInBlock = false;
        
        // Reset the player's BlockType to None as they are no longer near any block
        CachedPlayerCharacter->SetBlockType(EWellnessBlockType::None);
		CachedPlayerCharacter->GetMobileUIWidget()->InteractionWidget->SetWidgetDeepBreathingImage(); // Reset the mobile UI prompt to show deep breathing image

        // Clear the player reference.
        CachedPlayerCharacter = nullptr;
    }
}

/**
 *  Updates the vertical levitation position of the block based on the current MeditationBlockState.
 * Handles rising, floating up/down, and lowering states by adjusting the BlockMesh's relative Z location.
 * Only executes meaningful logic if the block type is Meditation and the state is not None.
 *  DeltaTime The time elapsed since the last frame.
 */
void AWellnessBlock::UpdateLevitation(float DeltaTime)
{
    // Ensure the mesh component is valid before trying to move it.
    if (!BlockMesh) return;

    // Get the current relative location of the mesh.
    FVector CurrentLocation = BlockMesh->GetRelativeLocation();
    float CurrentHeight = CurrentLocation.Z;
    float TargetHeight = CurrentHeight; // Initialize target height to current height
    float NewHeight = CurrentHeight; // Variable for calculating new height with constant speed

    // Determine the target height and apply movement based on the current state.
    switch (MeditationBlockState)
    {
    case EMeditationBlockState::Rising:
        TargetHeight = InitialLocation.Z + MaxDistanceFromGround;
        NewHeight = CurrentHeight + (RiseSpeed * DeltaTime);
        // Check if we've reached or passed the target height
        if (NewHeight >= TargetHeight)
        {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Reached max height"));
            MeditationBlockState = EMeditationBlockState::FloatingDown; // Transition to floating
            NewHeight = TargetHeight; // Snap to exact height
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Rising"));

        }
        break;

    case EMeditationBlockState::FloatingDown:
        TargetHeight = InitialLocation.Z + LowestDistanceFromGround;
        // Use constant speed movement instead of interpolation
        NewHeight = CurrentHeight - (LowerSpeed * DeltaTime);
        // Check if we've reached or passed the target height
        if (NewHeight <= TargetHeight)
        {
            MeditationBlockState = EMeditationBlockState::FloatingUp; // Reverse float direction
            NewHeight = TargetHeight;

        }
        break;

    case EMeditationBlockState::FloatingUp:
        TargetHeight = InitialLocation.Z + MaxDistanceFromGround;
        // Use constant speed movement instead of interpolation
        NewHeight = CurrentHeight + (RiseSpeed * DeltaTime);
        // Check if we've reached or passed the target height
        if (NewHeight >= TargetHeight)
        {
            MeditationBlockState = EMeditationBlockState::FloatingDown; // Reverse float direction
            NewHeight = TargetHeight;

        }
        break;

    case EMeditationBlockState::Lowering:
        TargetHeight = InitialLocation.Z; // Target the original resting height (Z=0 relative)
        // Use constant speed movement instead of interpolation
        NewHeight = CurrentHeight - (LowerSpeed * DeltaTime);
        // Check if we've reached or passed the target height
        if (NewHeight <= TargetHeight)
        {
            if (CachedPlayerCharacter)
            {
                MeditationBlockState = EMeditationBlockState::None; // Reached bottom, stop levitation
                CachedPlayerCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle); // Reset player state to Idle
                NewHeight = TargetHeight; // Snap to exact height

            }
        }
        break;

    case EMeditationBlockState::None:
    default:
        // Do nothing if in None state or an unexpected state.
        return; // Exit early, no need to set location
    }

    // Apply the new height to the mesh's relative location.
    BlockMesh->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewHeight));
}

