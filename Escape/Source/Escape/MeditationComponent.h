// MeditationComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "MeditationComponent.generated.h"

/**
 * Enum representing the possible states of the meditation mechanic.
 */
UENUM(BlueprintType)
enum class EMeditationState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),        // Not meditating
    Meditating  UMETA(DisplayName = "Meditating")  // Actively meditating
};

/**
 * A component that manages a meditation mechanic for a character.
 * Supports both timed meditation (with a duration) and toggle mode (duration = 0).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UMeditationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** Constructor for the meditation component. */
    UMeditationComponent();

    /**
     * Starts the meditation process if the current state is Idle.
     * If MeditationDuration > 0, sets a timer; if 0, acts as a toggle.
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartMeditation();
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void Meditation();
    /**
     * Stops the meditation process if the current state is Meditating.
     * Clears the timer if applicable and resets to Idle.
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StopMeditation();

    /**
     * Gets the current meditation state.
     * @return The current EMeditationState.
     */
    UFUNCTION(BlueprintPure, Category = "Meditation")
    EMeditationState GetMeditationState() const { return MeditationState; }

protected:
    /** Called when the component is initialized in the game world. */
    virtual void BeginPlay() override;

private:
    // --- Configuration Properties ---

    /** Duration of meditation in seconds. Set to 0 for toggle mode (no timer). */
    UPROPERTY(EditAnywhere, Category = "Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MeditationDuration = 10.0f;

    
    // --- Runtime State ---
    /** Keditation cue goes here*/
    UPROPERTY(EditAnywhere, Category = "Meditation")
    TObjectPtr<USoundCue> MeditationMusic;

    /** Current state of the meditation mechanic. */
    UPROPERTY(VisibleInstanceOnly, Category = "Meditation", Transient)
    EMeditationState MeditationState = EMeditationState::Idle;

    /** Timer tracking progress (unused in toggle mode). */
    UPROPERTY(VisibleInstanceOnly, Category = "Meditation", Transient)
    float MeditationTimer = 0.0f;

    /** Handle for the meditation timer (used when MeditationDuration > 0). */
    FTimerHandle MeditationTimerHandle;

    /** Audio component for playing and stopping meditation music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MeditationMusicComponent;
    // --- Internal Methods ---

    /**
     * Handles the start of meditation: sets state, plays animation/music, and optionally starts a timer.
     */
    void HandleMeditationStart();


    /**
     * Handles the end of meditation: stops animation/music and resets state.
     */
    void HandleMeditationStop();

    /**
     * Applies effects when meditation completes naturally (timer-based only).
     * Example: add calm effect
     */
    void ApplyMeditationEffects();

    /**
     * Called when the timer completes (if MeditationDuration > 0).
     * Applies effects and stops meditation.
     */
    void OnMeditationComplete();

    // --- Cached References ---

    /** Cached reference to the owning character for animation control. */
    TWeakObjectPtr<ACharacter> OwningCharacter;
};