#pragma once

#include "CoreMinimal.h"
#include "../Widgets/ScoreWidget.h"
#include "Components/ActorComponent.h"
#include "SecondCounterComponent.generated.h"


/**
 * USecondCounterComponent
 * A modular component that counts seconds of each activity 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API USecondCounterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Returns the current elapsed time
    UFUNCTION(BlueprintCallable, Category = "Counter")
    float GetElapsedTime() { return ElapsedTime; }

    UFUNCTION(BlueprintCallable, Category = "Counter")
    void UpdateElapsedTime(float ElapsedTimeP) { ElapsedTime += ElapsedTimeP; }

    void SetScoreWidget(UScoreWidget* ScoreWidgetp) {  
       ScoreWidget = ScoreWidgetp; 
    }
    // Sets default values for this component's properties
    USecondCounterComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Start the counter
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void StartCounter();

    // Stop the counter
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void StopCounter();

    // Reset the counter to zero
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void ResetCounter();


    // Function to stop the counter and save the score
    UFUNCTION(BlueprintCallable, Category = "Counter")
    void StopAndSaveScore();

    // Save slot name and user index for saving/loading the high score
    FString SaveSlotName;
    int32 UserIndex;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    // Whether the counter is currently running
    bool bIsCounting;

    // The elapsed time in seconds
    float ElapsedTime;

    TObjectPtr<class UScoreWidget> ScoreWidget;

    void CheckAndSaveHighScore(float CurrentScore);


    /** Cached reference to the owning character. */
    TWeakObjectPtr<ACharacter> OwningCharacter;
};
