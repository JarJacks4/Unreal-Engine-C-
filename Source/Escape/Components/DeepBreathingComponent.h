#pragma once
#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "Components/ActorComponent.h"
#include "DeepBreathingComponent.generated.h"
class AEscapeCharacter;

class UBreathingBubbleWidget;

UENUM(BlueprintType)
enum class EDeepBreathingStatus : uint8
{
    BreathingOut,
    BreathingIn
};

/**
 * Enum defining different types of breathing exercises.
 */
UENUM(BlueprintType)
enum class EBreathingType : uint8
{
    Deep            UMETA(DisplayName = "Deep Breathing"), // General deep breathing
    Basic           UMETA(DisplayName = "Basic Breathing"),
    BoxBreathing    UMETA(DisplayName = "Box Breathing"),
    FourSevenEight  UMETA(DisplayName = "4-7-8 Breathing")
};

/**
 *  UDeepBreathingComponent
 * A component responsible for managing the deep breathing exercise mini-game.
 * Manages the UI (BreathingBubbleWidget), the scaling of the breathing bubble,
 * timing, and audio feedback.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API UDeepBreathingComponent : public UActorComponent
{
   GENERATED_BODY()

public:
   /**  Constructor for the DeepBreathingComponent. */
   UDeepBreathingComponent();

   /**
    *  The SoundCue asset to play during the deep breathing exercise.
    *  Assign a music track or ambient sound to guide the player's breathing.
    */
   UPROPERTY(EditAnywhere, Category = "Deep Breathing")
   TObjectPtr<USoundCue> DeepBreathingMusic;

   /**
    *  Sets the current status of the deep breathing exercise.
    *  PlayerDeepBreathingStatus The new breathing status (BreathingIn or BreathingOut).
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void SetDeepBreathingStatus(EDeepBreathingStatus PlayerDeepBreathingStatus) { DeepBreathingStatus = PlayerDeepBreathingStatus; };

   /**
    *  Gets the current status of the deep breathing exercise.
    * @return The current breathing status (BreathingIn or BreathingOut).
    */
   UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Deep Breathing")
   EDeepBreathingStatus GetDeepBreathingStatus() const { return DeepBreathingStatus; };

   /**
    *  Called every frame if the component is active.
    * Updates the breathing bubble's scale when the exercise is active.
    *  DeltaTime Time since the last frame.
    *  TickType Type of tick (e.g., game tick).
    *  ThisTickFunction The function being ticked.
    */
   virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

   /**
    *  Starts the deep breathing exercise.
    * Displays the breathing bubble widget, plays the music, and initiates the inhale/exhale cycle.
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void StartDeepBreathing();

   /**
    *  Stops the deep breathing exercise.
    * Removes the breathing bubble widget, stops the music, and resets the state.
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void StopDeepBreathing();

   /**
    *  Called when the player taps the breathing bubble.
    * Determines if the tap was within the correct timing window and adjusts the exercise accordingly.
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void OnBreathingBubbleTapped();

   /**
    *  Sets the deep breathing duration in seconds.
    *  @param Duration The new duration in seconds (0 for toggle mode)
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void SetDeepBreathingDuration(float Duration);
   
   /**
    *  Gets the current deep breathing duration setting.
    *  @return The deep breathing duration in seconds (0 for toggle mode)
    */
   UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Deep Breathing")
   float GetDeepBreathingDuration() const { return DeepBreathingDuration; }
   
   /**
    *  Gets the current deep breathing timer value (time elapsed since exercise started).
    *  @return The elapsed time in seconds
    */
   UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Deep Breathing")
   float GetDeepBreathingTimer() const { return DeepBreathingTimer; }
   
   /**
    *  Clears the deep breathing timer, resetting elapsed time to zero.
    *  Useful when stopping the exercise prematurely.
    */
   UFUNCTION(BlueprintCallable, Category = "Deep Breathing")
   void ClearDeepBreathingTimer();
   
   /**
    *  Checks if the deep breathing timer is currently active.
    *  @return True if deep breathing timer is running
    */
   UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Deep Breathing")
   bool IsDeepBreathingTimerActive() const;

   /** Sets the BreathingBubbleWidget reference. */
   void SetBreathingBubbleWidget(UBreathingBubbleWidget* Widget) { BreathingBubbleWidget = Widget; }
   /** Gets the BreathingBubbleWidget reference. */
   UBreathingBubbleWidget* GetBreathingBubbleWidget() const { return BreathingBubbleWidget; }

   /** Default completion points, can be overridden by specific breathing types. */
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ToolTip = "Default points awarded for completing the duration. Specific types might override this."))
   int32 DefaultCompletionPoints = 3; // Default points for Deep Breathing

   /** The specific type of breathing exercise to perform. */
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deep Breathing")
   EBreathingType BreathingType = EBreathingType::Deep; // Default to general Deep Breathing

private:
   /**  Audio component for playing and stopping Deep Breathing music. */
   UPROPERTY(Transient)
   TObjectPtr<UAudioComponent> DeepBreathingMusicComponent;

   /**  The current status of the deep breathing exercise (BreathingIn or BreathingOut). */
   EDeepBreathingStatus DeepBreathingStatus;

   /**
    *  Updates the breathing bubble's scale based on the current state and time.
    *  DeltaTime Time since the last frame.
    */
   void UpdateBreathingBubble(float DeltaTime);

   /**  The current scale of the breathing bubble (0.5 to 1.5). */
   float BubbleScale;

   /**  Scaling direction (1 for expanding, -1 for shrinking). */
   float ScaleDirection;
   
   /**
    *  Duration of deep breathing exercise in seconds. Set to 0 for toggle mode (no timer).
    * If > 0, the exercise will automatically stop after this time.
    * If = 0, the exercise will continue until stopped manually.
    * Default is 300 seconds (5 minutes) as specified in deep breathing documentation.
    */
   UPROPERTY(EditAnywhere, Category = "Deep Breathing", meta = (ClampMin = "0.0", UIMin = "0.0", ToolTip = "Default duration in seconds. Specific types might override this."))
   float DeepBreathingDuration = 300.0f; // 5 minutes for Deep Breathing

   /**  Timer tracking progress (unused in toggle mode). */
   UPROPERTY(VisibleInstanceOnly, Category = "Deep Breathing", Transient)
   float DeepBreathingTimer = 0.0f;

   /**  Handle for the deep breathing timer (used when DeepBreathingDuration > 0). */
   FTimerHandle DeepBreathingTimerHandle;
   
   /**
    *  Called when the timer completes (if `DeepBreathingDuration` > 0).
    * Applies effects and stops the exercise.
    */
   void OnDeepBreathingComplete();
   
   /**
    *  Applies effects when deep breathing completes naturally (timer-based only).
    * Example: add calm effect or stress reduction
    */
   void ApplyDeepBreathingEffects();

   /**  Cached reference to the owning character for efficient access. */
   TWeakObjectPtr<ACharacter> OwningCharacter;
   /**  Cached reference to the owning EscapeCharacter for efficient access. */
   AEscapeCharacter* CachedEscapeCharacter = nullptr;

   /** Reference to the breathing bubble widget. */
   UBreathingBubbleWidget* BreathingBubbleWidget = nullptr;
};
