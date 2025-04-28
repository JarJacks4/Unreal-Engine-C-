#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/CapsuleComponent.h>
#include "WellnessBlock.generated.h"

/**
 * Enum defining the type of wellness block.
 * - Stretching: Triggers a stretching activity.
 * - Meditation: Triggers a meditation activity.
 */
UENUM(BlueprintType)
enum class EWellnessBlockType : uint8
{
    Stretching,
    Meditation,
    None
};

UENUM(BlueprintType)
enum class EMeditationBlockState : uint8
{
    None,
    Rising,
    FloatingUp,
    FloatingDown,
    Lowering
};
/**
 * Base class for in-world interactive wellness blocks.
 * Detects player overlap to show interaction UI and manages visual effects like levitation for meditation blocks.
 */
UCLASS()
class ESCAPE_API AWellnessBlock : public AActor
{
    GENERATED_BODY()
public:
    // Constructor
    AWellnessBlock();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float RotationSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MaxRotationAngle = 45.0f;

    UFUNCTION(BlueprintCallable, Category = "Rotation")
    float GetCurrentRotation() const { return CurrentRotation; }


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mobile UI Widget")
    UCapsuleComponent* SceneRoot;
    UFUNCTION(BlueprintCallable)
    void SetMeditationBlockState(EMeditationBlockState MeditationBlock) { MeditationBlockState = MeditationBlock; };
    UPROPERTY(VisibleAnywhere)
    TWeakObjectPtr<ACharacter> PlayerRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Meditation")
    float MaxDistanceFromGround;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float LowestDistanceFromGround;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float MovementSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float LowerSpeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float RiseSpeed;

    /**
     * Called every frame to update the levitation effect for meditation blocks.
     */
    virtual void Tick(float DeltaTime) override;
    
protected:
    virtual void BeginPlay() override;

    /**
     * Called when the player overlaps with the block's trigger volume.
     */
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
     * Called when the player stops overlapping with the block's trigger volume.
     */
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Mesh component for the block's visual representation
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BlockMesh;
    
    // Trigger volume for detecting player overlap
    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* TriggerVolume;

    // Type of wellness block (Stretching or Meditation)
    UPROPERTY(EditAnywhere)
    EWellnessBlockType BlockType;

    // Current levitation offset for meditation blocks
    float LevitationOffset;

    // Timer handle for levitation updates
    FTimerHandle LevitationTimerHandle;

private:
    // Internal variables
    float CurrentRotation;
    float TargetRotation;
    bool bIsRotatingRight;

    // Phone sensor data
    float GetDeviceTilt() const;

    // Rotation handling
    void UpdateRotation(float DeltaTime);

    EMeditationBlockState MeditationBlockState;
    /**
     * Updates the levitation position of the meditation block.
     */
    void UpdateLevitation(float DeltaTime);

   
};