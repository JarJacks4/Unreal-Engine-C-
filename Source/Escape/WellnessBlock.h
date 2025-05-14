#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "InputActionValue.h"
#include "WellnessBlock.generated.h"

class AEscapeCharacter;
class UMobileUIWidget;

UENUM(BlueprintType)
enum class EWellnessBlockType : uint8
{
    Stretching UMETA(DisplayName = "Stretching"),
    Meditation UMETA(DisplayName = "Meditation"),
    DeepBreathing UMETA(DisplayName = "Deep Breathing"),
    Journaling UMETA(DisplayName = "Journaling"),
    None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EMeditationBlockState : uint8
{
    None UMETA(DisplayName = "None"),
    Rising UMETA(DisplayName = "Rising"),
    FloatingUp UMETA(DisplayName = "Floating Up"),
    FloatingDown UMETA(DisplayName = "Floating Down"),
    Lowering UMETA(DisplayName = "Lowering")
};

UCLASS()
class ESCAPE_API AWellnessBlock : public AActor
{
    GENERATED_BODY()
public:
    AWellnessBlock();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation")
    FVector InitialLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation")
    float RotationSpeed = 100.0f;

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationBlockState(EMeditationBlockState NewMeditationBlockState) { MeditationBlockState = NewMeditationBlockState; };

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    EMeditationBlockState GetMeditationBlockState() { return MeditationBlockState; };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Rotation")
    float MaxRotationAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Visuals|Meditation")
    float MaxDistanceFromGround = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation")
    float LowestDistanceFromGround = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation")
    float MovementSpeed = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation")
    float LowerSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Meditation")
    float RiseSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wellness")
    EWellnessBlockType BlockType = EWellnessBlockType::None;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> SceneRootComponent;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BlockMesh;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> TriggerVolume;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Visuals|Rotation")
    float GetCurrentRotation() const { return CurrentRotation; }

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction", Transient)
    TWeakObjectPtr<ACharacter> PlayerRef;

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float CurrentRotation = 0.0f;
    float TargetRotation = 0.0f;
    bool bIsRotatingRight = true;

    EMeditationBlockState MeditationBlockState = EMeditationBlockState::None;

    AEscapeCharacter* CachedPlayerCharacter = nullptr;

    void UpdateRotation(float DeltaTime);
    void ApplyRotation();
    void UpdateLevitation(float DeltaTime);
};
