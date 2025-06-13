#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "../Widgets/RythmWidget.h" // Include for EStretchState enum definition
#include "StretchingComponent.generated.h"
class AEscapeCharacter;

/**
 *  UStretchingComponent
 * A component that manages stretching exercises for a character.
 * Supports both timed stretching (with a duration) and toggle mode (duration = 0).
 * It handles music, timing, and effects related to stretching routines.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UStretchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**  Constructor for the stretching component. */
    UStretchingComponent();

    /**
     *  Starts the stretching process if the current state is Idle.
     * If `StretchingDuration` > 0, sets a timer; if 0, acts as a toggle.
     */
    UFUNCTION(BlueprintCallable, Category = "Stretching")
    void StartStretching();

    /**
     *  Stops the stretching process if the current state is Stretching.
     * Clears the timer if applicable and resets to Idle.
     */
    UFUNCTION(BlueprintCallable, Category = "Stretching")
    void StopStretching();
    
    /**
     *  Sets the current stretch state/pose based on user input.
     *  @param NewStretchState The stretching state to transition to
     */
    UFUNCTION(BlueprintCallable, Category = "Stretching")
    void SetStretchState(EStretchState NewStretchState);
    
    /**
     *  Gets the current stretch state/pose.
     *  @return The current stretch state enum value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stretching")
    EStretchState GetStretchState() const { return CurrentStretchState; }
    
    /**
     *  Gets the current stretch state/pose.
     *  Used by RythmWidget to check player's current pose.
     *  @return The current stretch state enum value
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stretching")
    EStretchState GetCurrentStretchState() const { return CurrentStretchState; }
    
    /**
     *  Sets the stretching duration in seconds.
     *  @param Duration The new stretching duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, Category = "Stretching")
    void SetStretchingDuration(float Duration);
    
    /**
     *  Gets the current stretching duration setting.
     *  @return The stretching duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stretching")
    float GetStretchingDuration() const { return StretchingDuration; }
    
    /**
     *  Gets the current stretching timer value (time elapsed since stretching started).
     *  @return The elapsed stretching time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stretching")
    float GetStretchingTimer() const { return StretchingTimer; }
    
    /**
     *  Clears the stretching timer, resetting elapsed time to zero.
     *  Useful when stopping stretching prematurely.
     */
    UFUNCTION(BlueprintCallable, Category = "Stretching")
    void ClearStretchingTimer();
    
    /**
     *  Checks if the stretching timer is currently active.
     *  @return True if stretching timer is running
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stretching")
    bool IsStretchingTimerActive() const;

    /** Duration of stretching in seconds. Set to 0 for toggle mode (no timer). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stretching", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float StretchingDuration = 300.0f;

    /** Points awarded for completing the stretching activity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints = 40;

    /** Minimum points awarded for this activity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints = 1;

    /** Completion and minimum points for Stretching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints_Stretching = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints_Stretching = 1;

    /** Detraction points for stretching activity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0", UIMin = "0"))
    int32 DetractionPoints_Stretching = 0;

protected:
    /** 
     *  Called when the component is initialized in the game world.
     * Caches a reference to the owning character.
     */
    virtual void BeginPlay() override;

private:
    /**
     *  The current stretch state/pose of the character.
     */
    UPROPERTY(VisibleInstanceOnly, Category = "Stretching", Transient)
    EStretchState CurrentStretchState = EStretchState::StretchLeft;

    /**
     *  The SoundCue asset to play during the stretching exercise.
     * Assign a music track or ambient sound to enhance the experience.
     */
    UPROPERTY(EditAnywhere, Category = "Stretching")
    TObjectPtr<USoundCue> StretchingMusic;

    /**  Timer tracking progress (unused in toggle mode). */
    UPROPERTY(VisibleInstanceOnly, Category = "Stretching", Transient)
    float StretchingTimer = 0.0f;

    /**  Handle for the stretching timer (used when StretchingDuration > 0). */
    FTimerHandle StretchingTimerHandle;

    /**  Audio component for playing and stopping stretching music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> StretchingMusicComponent;

    /**
     *  Handles the start of stretching: sets state, plays music, and optionally starts a timer.
     */
    void HandleStretchingStart();

    /**
     *  Handles the end of stretching: stops music and resets state.
     */
    void HandleStretchingStop();

    /**
     *  Applies effects when stretching completes naturally (timer-based only).
     * Example: add energized effect or flexibility boost
     */
    void ApplyStretchingEffects();

    /**
     *  Called when the timer completes (if `StretchingDuration` > 0).
     * Applies effects and stops stretching.
     */
    void OnStretchingComplete();

    /**  Cached reference to the owning EscapeCharacter for efficient access. */
    AEscapeCharacter* CachedEscapeCharacter = nullptr;
};
