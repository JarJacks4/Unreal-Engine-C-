#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "StretchingComponent.generated.h"

class UAnimMontage;

/**
 * Component that handles stretching animations for the player character.
 * Activated when interacting with stretching blocks in the game world.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UStretchingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // --- Runtime State ---
    /** Stretching cue goes here*/
    UPROPERTY(EditAnywhere, Category = "Stretching")
    TObjectPtr<USoundCue> StretchingMusic;
    UStretchingComponent();
    /**
     * Starts the stretching activity.
     */
    UFUNCTION(BlueprintCallable)
    void StartStretching();

    /**
     * Stops the stretching activity
     */
    UFUNCTION(BlueprintCallable)
    void StopStretching();



protected:
    void BeginPlay();

private:
    /** Audio component for playing and stopping Stretching music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> StretchingMusicComponent;
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
    // --- Cached References ---

/** Cached reference to the owning character */
    TWeakObjectPtr<ACharacter> OwningCharacter;
};