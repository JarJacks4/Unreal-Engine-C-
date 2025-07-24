#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "../WellnessBlock.h"
#include "MeditationComponent.generated.h"
class AEscapeCharacter;

/**
 * Enum defining different types of meditation practices.
 */
UENUM(BlueprintType)
enum class EMeditationType : uint8
{
    Guided          UMETA(DisplayName = "Guided Meditation"),
    Mindfulness     UMETA(DisplayName = "Mindfulness Meditation"),
    Cosmic          UMETA(DisplayName = "Cosmic Meditation"),
    MeditationPad   UMETA(DisplayName = "Meditation Pad")
};

/**
 *  UMeditationComponent
 * A component that manages a meditation mechanic for a character.
 * Supports both timed meditation (with a duration) and toggle mode (duration = 0).
 * It handles music, timing, and effects related to meditation.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UMeditationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**  Constructor for the meditation component. */
    UMeditationComponent();

    /**
     *  Starts the meditation process if the current state is Idle.
     * If `MeditationDuration` > 0, sets a timer; if 0, acts as a toggle.
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartMeditation();

    /**
     *  Stops the meditation process if the current state is Meditating.
     * Clears the timer if applicable and resets to Idle.
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StopMeditation();

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
     *  Sets the meditation duration in seconds.
     *  @param Duration The new meditation duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationDuration(float Duration);
    
    /**
     *  Gets the current meditation duration setting.
     *  @return The meditation duration in seconds (0 for toggle mode)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    float GetMeditationDuration() const { return MeditationDuration; }
    
    /**
     *  Gets the current meditation timer value (time elapsed since meditation started).
     *  @return The elapsed meditation time in seconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    float GetMeditationTimer() const { return MeditationTimer; }
    
    /**
     *  Clears the meditation timer, resetting elapsed time to zero.
     *  Useful when stopping meditation prematurely.
     */
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ClearMeditationTimer();
    
    /**
     *  Checks if the meditation timer is currently active.
     *  @return True if meditation timer is running
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    bool IsMeditationTimerActive() const;

    /** Completion and minimum points for Guided Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints_Guided = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints_Guided = 1;
    /** Detraction points for Guided Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0", UIMin = "0"))
    int32 DetractionPoints_Guided = 0;
    /** Completion and minimum points for Mindfulness Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints_Mindfulness = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints_Mindfulness = 1;
    /** Detraction points for Mindfulness Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0", UIMin = "0"))
    int32 DetractionPoints_Mindfulness = 0;
    /** Completion and minimum points for Cosmic Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints_Cosmic = 15;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints_Cosmic = 1;
    /** Detraction points for Cosmic Meditation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0", UIMin = "0"))
    int32 DetractionPoints_Cosmic = 0;
    /** Completion and minimum points for Meditation Pad */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 CompletionPoints_MeditationPad = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "1", UIMin = "1"))
    int32 MinimumPoints_MeditationPad = 1;
    /** Detraction points for Meditation Pad */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0", UIMin = "0"))
    int32 DetractionPoints_MeditationPad = 0;
    /**
    *  Duration of meditation in seconds. Set to 0 for toggle mode (no timer).
    * If > 0, the meditation will automatically stop after this time.
    * If = 0, meditation will continue until stopped manually.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MeditationDuration = 300.0f;
    /** The specific type of meditation to perform. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    EMeditationType MeditationType = EMeditationType::Guided; // Default to Guided

protected:
    /** 
     *  Called when the component is initialized in the game world.
     * Caches a reference to the owning character.
     */
    virtual void BeginPlay() override;

private:

    UPROPERTY(Transient) // Transient, created at runtime
    TObjectPtr<AWellnessBlock> BlockRef; // Reference to the block for interaction


   

    /** SoundCue for Guided Meditation. */
    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> GuidedMeditationMusic;

    /** SoundCue for Mindfulness Meditation. */
    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MindfulnessMeditationMusic;

    /** SoundCue for Cosmic Meditation. */
    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> CosmicMeditationMusic;

    /** SoundCue for Meditation Pad. */
    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MeditationPadMusic;

    /** SoundCue for Meditation Pad. */
    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MeditationMusic;

    /**  Timer tracking progress (unused in toggle mode). */
    UPROPERTY(VisibleInstanceOnly, Category = "Meditation", Transient)
    float MeditationTimer = 0.0f;

    /**  Handle for the meditation timer (used when MeditationDuration > 0). */
    FTimerHandle MeditationTimerHandle;

    /**  Audio component for playing and stopping meditation music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MeditationMusicComponent;

    /**
     *  Handles the start of meditation: sets state, plays music, and optionally starts a timer.
     */
    void HandleMeditationStart();

    /**
     *  Handles the end of meditation: stops music and resets state.
     */
    void HandleMeditationStop();

    /**
     *  Applies effects when meditation completes naturally (timer-based only).
     * Example: add calm effect
     */
    void ApplyMeditationEffects();

    /**
     *  Called when the timer completes (if `MeditationDuration` > 0).
     * Applies effects and stops meditation.
     */
    void OnMeditationComplete();


    /**  Cached reference to the owning EscapeCharacter for efficient access. */
    AEscapeCharacter* CachedEscapeCharacter = nullptr;
};
