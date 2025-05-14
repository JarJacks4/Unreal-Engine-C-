#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "../WellnessBlock.h" // Assuming Meditation Pad uses WellnessBlock too
#include "MeditationPadComponent.generated.h" // Changed generated header name
class AEscapeCharacter;

/**
 *  UMeditationPadComponent
 * A component that manages a meditation pad mechanic for a character.
 * Supports both timed interaction (with a duration) and toggle mode (duration = 0).
 * It handles music, timing, and effects related to the meditation pad.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UMeditationPadComponent : public UActorComponent // Changed class name
{
    GENERATED_BODY()

public:
    /**  Constructor for the meditation pad component. */
    UMeditationPadComponent(); // Changed constructor name

    /**
     *  Starts the meditation pad interaction if the current state is Idle.
     * If `InteractionDuration` > 0, sets a timer; if 0, acts as a toggle.
     */
    UFUNCTION(BlueprintCallable, Category = "MeditationPad") // Changed category
    void StartMeditation(); // Changed function name

    /**
     *  Stops the meditation pad interaction if the current state is Interacting.
     * Clears the timer if applicable and resets to Idle.
     */
    UFUNCTION(BlueprintCallable, Category = "MeditationPad") // Changed category
    void StopMeditation(); // Changed function name

    /**
     *  Sets the reference to the block that this component is associated with.
     *  GottenBlock The block reference to set.
     */
    UFUNCTION(BlueprintCallable)
    void SetBlockRef(AWellnessBlock* GottenBlock) { BlockRef = GottenBlock; };

    /**
     *  Gets the reference to the block that this component is associated with.
     *  Returns the current block reference.
     */
    UFUNCTION(BlueprintCallable)
    AWellnessBlock* GetBlockRef() { return BlockRef; };

    /**
     *  Sets the interaction duration in seconds.
     *  @param Duration The new interaction duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, Category = "MeditationPad") // Changed category
        void SetMeditationDuration(float Duration); // Changed function name

    /**
     *  Gets the current interaction duration setting.
     *  @return The interaction duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MeditationPad") // Changed category
    float GetMeditationDuration() const { return MeditationDuration; } // Changed function name and variable

    /**
     *  Gets the current interaction timer value (time elapsed since interaction started).
     *  @return The elapsed interaction time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MeditationPad") // Changed category
    float GetMeditationTimer() const { return MeditationTimer; } // Changed function name and variable

    /**
     *  Clears the interaction timer, resetting elapsed time to zero.
     *  Useful when stopping interaction prematurely.
     */
    UFUNCTION(BlueprintCallable, Category = "MeditationPad") // Changed category
    void ClearMeditationTimer(); // Changed function name

    /**
     *  Checks if the interaction timer is currently active.
     *  @return True if interaction timer is running
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MeditationPad") // Changed category
    bool IsMeditationTimerActive() const; // Changed function name

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    int32 CompletionPoints = 10; // Kept scoring variable

protected:
    /**
     *  Called when the component is initialized in the game world.
     * Caches a reference to the owning character.
     */
    virtual void BeginPlay() override;

private:

    UPROPERTY(Transient) // Transient, created at runtime
    TObjectPtr<AWellnessBlock> BlockRef; // Reference to the block for interaction


    /**
     *  Duration of interaction in seconds. Set to 0 for toggle mode (no timer).
     * If > 0, the interaction will automatically stop after this time.
     * If = 0, interaction will continue until stopped manually.
     */
    UPROPERTY(EditAnywhere, Category = "MeditationPad", meta = (ClampMin = "0.0", UIMin = "0.0")) // Changed category
    float MeditationDuration = 10.0f; // Changed variable name

    /**
     *  The SoundCue asset to play during the interaction exercise.
     * Assign a music track or ambient sound to enhance the experience.
     */
    UPROPERTY(EditAnywhere, Category = "MeditationPad") // Changed category
    TObjectPtr<USoundCue> MeditationMusic; // Changed variable name

    /**  Timer tracking progress (unused in toggle mode). */
    UPROPERTY(VisibleInstanceOnly, Category = "MeditationPad", Transient) // Changed category
    float MeditationTimer = 0.0f; // Changed variable name

    /**  Handle for the interaction timer (used when InteractionDuration > 0). */
    FTimerHandle MeditationTimerHandle; // Changed variable name

    /**  Audio component for playing and stopping interaction music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MeditationMusicComponent; // Changed variable name

    /**
     *  Handles the start of interaction: sets state, plays music, and optionally starts a timer.
     */
    void HandleMeditationStart(); // Changed function name

    /**
     *  Handles the end of interaction: stops music and resets state.
     */
    void HandleMeditationStop(); // Changed function name

    /**
     *  Applies effects when interaction completes naturally (timer-based only).
     * Example: add calm effect
     */
    void ApplyMeditationEffects(); // Changed function name

    /**
     *  Called when the timer completes (if `InteractionDuration` > 0).
     * Applies effects and stops interaction.
     */
    void OnMeditationComplete(); // Changed function name

    /**  Cached reference to the owning EscapeCharacter for efficient access. */
    AEscapeCharacter* CachedEscapeCharacter = nullptr;
};
