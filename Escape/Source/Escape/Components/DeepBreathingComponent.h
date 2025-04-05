#pragma once  
#include "CoreMinimal.h"  
#include "Sound/SoundCue.h"
#include "Components/ActorComponent.h"  
#include "DeepBreathingComponent.generated.h"  

class UBreathingBubbleWidget;  
UENUM(BlueprintType)  
enum class EDeepBreathingStatus : uint8  
{  
   BreathingOut,  
   BreathingIn  
};  
/**  
* Component responsible for managing the breathing exercise, including the UI and bubble scaling logic.  
*/  
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))  
class ESCAPE_API UDeepBreathingComponent : public UActorComponent  
{  
   GENERATED_BODY()  
public:  
   // Constructor  
   UDeepBreathingComponent();  
   /** Deep Breathing cue goes here*/
   UPROPERTY(EditAnywhere, Category = "Deep Breathing")
   TObjectPtr<USoundCue> DeepBreathingMusic;


   UFUNCTION(BlueprintCallable)
   void SetDeepBreathingStatus(EDeepBreathingStatus PlayerDeepBreathingStatus) { DeepBreathingStatus = PlayerDeepBreathingStatus; };

   UFUNCTION(BlueprintCallable)
   EDeepBreathingStatus GetDeepBreathingStatus() { return DeepBreathingStatus; };
   /**  
    * Updates the breathing bubble's scale every frame when the exercise is active.  
    */  
   virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;  

   /**  
    * Starts the breathing exercise and displays the breathing bubble widget.  
    */  
   UFUNCTION(BlueprintCallable)  
   void StartDeepBreathing();  

   /**  
    * Stops the breathing exercise and removes the breathing bubble widget.  
    */  
   UFUNCTION(BlueprintCallable)  
   void StopDeepBreathing();  

   /**  
    * Handles tap events on the breathing bubble, checking if the tap timing is correct.  
    */  
   UFUNCTION(BlueprintCallable)
   void OnBreathingBubbleTapped();  


private:  
    /** Audio component for playing and stopping Deep Breathing music. */
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> DeepBreathingMusicComponent;
   EDeepBreathingStatus DeepBreathingStatus;  
   /**  
    * Updates the breathing bubble's scale based on elapsed time.  
    * @param DeltaTime Time since the last frame.  
    */  
   void UpdateBreathingBubble(float DeltaTime);  

   // Current scale of the breathing bubble (0.5 to 1.5)  
   float BubbleScale;  

   // Scaling direction (1 for expanding, -1 for shrinking)  
   float ScaleDirection;  

   // --- Cached References ---  

/** Cached reference to the owning character */  
   TWeakObjectPtr<ACharacter> OwningCharacter;  
};
