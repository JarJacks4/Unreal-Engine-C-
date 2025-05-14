#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SecondCounterComponent.generated.h" 


class UScoreWidget;
class ACharacter;
class UHighScoreSaveGame;


/**
 *  USecondCounterComponent
 * A modular component designed to track elapsed time in seconds for various wellness activities.
 * It integrates with a UScoreWidget to display the time and can save the final time
 * as a high score using Unreal's SaveGame system.
 * 
 * This component forms the foundation of the game's activity tracking and points allocation system:
 * 
 * Meditation Activities:
 * - Guided Meditation: 5 points upon completion
 * - Mindfulness Meditation: 10 points for 15-minute sessions
 * - Cosmic Meditation: 50 points for engaging in self-healing breathing meditation
 * - Deep Breathing Exercises: 3 points for 5-minute sessions
 * - Basic Breathing Exercises: 10 points for introductory breathing techniques
 * 
 * Self-Care Activities:
 * - Journaling: 5 points for 10-minute writing sessions
 * - Positive Affirmations: 2 points for reciting 5 affirmations
 * - Stretching: 4 points for 10-minute routines
 * - Meditation on a Meditation Pad: 10 points in serene environments
 * 
 * Physical Activities:
 * - Short Walk: 5 points for 15-minute walks in serene environments
 * - Workout Session: 10 points for 30-minute structured workouts
 * - Yoga/Pilates on chosen Yoga Mat: 15 points for completing pose courses
 * 
 * Creative Activities:
 * - Creative Drawing or Crafting: 6 points for 20-minute creative sessions
 * - Cooking a Healthy Meal: 8 points for preparing nutritious meals
 * 
 * Additional bonus points are awarded for streaks and community engagement:
 * - 3-Day Streak Bonus: 15 points
 * - Weekly Streak: 50 points
 * - Sharing Experiences: 5 points
 * - Community Challenge Participation: 10 points
 * 
 * Points can be redeemed for rewards:
 * - 100 Points: Avatar customization options
 * - 200 Points: Exclusive guided meditation sessions
 * - 300 Points: Discounts on premium content
 * - 500 Points: Special self-care locations or activities
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API USecondCounterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**  Constructor for USecondCounterComponent. Initializes default values. */
    USecondCounterComponent();

    /**
     *  Returns the current elapsed time tracked by the counter.
     * @return The total time in seconds since the counter started or was last reset.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Counter")
    float GetElapsedTime() const { return ElapsedTime; }

    /**
     *  Manually adds or subtracts time from the current elapsed time.
     * Useful for applying bonuses or penalties during an activity.
     *  ElapsedTimeP The amount of time (in seconds) to add (positive value) or subtract (negative value).
     */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void UpdateElapsedTime(float ElapsedTimeP);

    /**
     *  Sets the UScoreWidget instance that this component will update.
     * This links the counter's value to a UI element for display.
     *  ScoreWidgetp Pointer to the UScoreWidget instance.
     */
    void SetScoreWidget(UScoreWidget* ScoreWidgetp) { ScoreWidget = ScoreWidgetp; }

    /**
     *  Called every frame if the component is ticking.
     * Increments the ElapsedTime if the counter is active (bIsCounting is true).
     *  DeltaTime Time elapsed since the last frame.
     *  TickType The type of tick.
     *  ThisTickFunction Internal tick function structure.
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     *  Starts the counter.
     * Sets bIsCounting to true and makes the associated ScoreWidget visible.
     */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void StartCounter();

    /**
     *  Stops the counter.
     * Sets bIsCounting to false. Does not reset the time or save the score.
     */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void StopCounter();

    /**
     *  Resets the elapsed time to zero.
     * Does not stop the counter if it's running.
     */
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void ResetCounter();

    /**
     *  The name of the save game slot used to store the high score for this counter.
     * Should be unique for each type of activity being scored (e.g., "MeditationScore", "BreathingScore").
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    FString SaveSlotName;

    /**
     *  The user index associated with the save game slot. Typically 0 for single-player games.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Game")
    int32 UserIndex = 0;
    
    /**
     *  Target time in seconds for this activity. When reached, the activity is considered complete.
     *  Can be customized for different activities (e.g., 300 seconds for 5-minute meditation, 
     *  900 seconds for 15-minute mindfulness sessions, etc.)
     */
    float TargetTime = 0.0f;
    
    /**
     *  Points awarded upon successful completion of this activity.
     *  Values vary based on activity type as specified in class documentation.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity Settings")
    int32 CompletionPoints = 5;
    
    /**
     *  Returns whether the activity has reached its target time.
     *  @return True if ElapsedTime is greater than or equal to TargetTime.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity")
    bool IsActivityComplete() const { return ElapsedTime >= TargetTime; }
    
    /**
     *  Returns the progress towards the target time as a percentage (0.0 to 1.0).
     *  @return The ratio of ElapsedTime to TargetTime, clamped between 0 and 1.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity")
    float GetProgressPercentage() const;
    
    /**
     *  Returns the time remaining until the target time is reached.
     *  @return The difference between TargetTime and ElapsedTime, or 0 if target is reached.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Activity")
    float GetTimeRemaining() const;

    UScoreWidget* GetScoreWidget() const { return ScoreWidget; }
    float GetTargetTime() const { return TargetTime; }
    int32 GetCompletionPoints() const { return CompletionPoints; }

protected:
    /**
     *  Called when the game starts or when the component is spawned.
     * Caches the owning character.
     */
    virtual void BeginPlay() override;

private:
    /**  Flag indicating whether the counter is currently active and incrementing time. */
    bool bIsCounting;

    /**  The accumulated elapsed time in seconds. */
    float ElapsedTime;

    /**  The high score for this activity, loaded from save or updated if beaten. */
    float HighScore = 0.0f;

    /**  Pointer to the UScoreWidget instance used to display the current time/score. Needs to be set via SetScoreWidget. */
    UPROPERTY(Transient)
    TObjectPtr<UScoreWidget> ScoreWidget;

    /**
     *  Checks the current score against the saved high score and updates the save file if necessary.
     *  CurrentScore The score (elapsed time) to compare against the saved high score.
     */
    void CheckAndSaveHighScore(float CurrentScore);

    /**  Cached weak pointer reference to the owning character. */
    TWeakObjectPtr<ACharacter> OwningCharacter;

    void ClampElapsedTime();

    bool bHasStopped = false;
};
