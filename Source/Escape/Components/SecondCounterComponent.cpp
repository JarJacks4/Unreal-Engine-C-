#include "SecondCounterComponent.h"
#include "../SaveGame/HighScoreSaveGame.h" // Include the specific SaveGame class header
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h" // Although TextBlock isn't directly used here, ScoreWidget uses it.
#include "../EscapeCharacter.h" // Include character header for casting owner
#include "../Widgets/ScoreWidget.h" // Include ScoreWidget header
#include "GameFramework/Character.h" // Include base Character header
#include "../WellnessSaveGame.h" // Include WellnessSaveGame header
#include "../Components/MeditationComponent.h"
#include "../Components/StretchingComponent.h"
#include "../Components/DeepBreathingComponent.h"

// Helper function to print debug messages if OnScreenDebugBool is true
static void PrintScoreDebugMessage(AEscapeCharacter* EscapeChar, const FString& Msg, FColor Color)
{
    if (EscapeChar && EscapeChar->OnScreenDebugBool && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, Color, Msg);
    }
}

/**
 *  Constructor for USecondCounterComponent.
 * Initializes the component to not be counting initially, elapsed time to zero,
 * and enables ticking so it can increment time when started.
 * 
 * This component tracks wellness activities and assigns points based on completion:
 * - Meditation activities range from 3 points (Deep Breathing) to 50 points (Cosmic Meditation)
 * - Self-Care activities range from 2 points (Positive Affirmations) to 10 points (Meditation Pad)
 * - Physical activities range from 5 points (Short Walk) to 15 points (Yoga/Pilates)
 * - Creative activities range from 6 points (Drawing) to 8 points (Cooking)
 */
USecondCounterComponent::USecondCounterComponent()
{
    PrimaryComponentTick.bCanEverTick = true; // Enable ticking to update time
    bIsCounting = false; // Start in a stopped state
    ElapsedTime = 0.0f; // Initialize time to zero
    TimerWidget = nullptr; // Initialize timer widget pointer
    CachedEscapeCharacter = nullptr; // Initialize character pointer
}

/**
 *  Called when the game starts or when the component is spawned.
 * Caches a weak pointer to the owning actor, cast to ACharacter.
 * Sets up the component to track activity time for subsequent point allocation.
 */
void USecondCounterComponent::BeginPlay()
{
    Super::BeginPlay();
    // Cache the owning character for potential future use (though not currently used elsewhere in this component)
    CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
    // Load high score from save game at startup
    if (!SaveSlotName.IsEmpty())
    {
        UHighScoreSaveGame* SaveGameInstance = Cast<UHighScoreSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
        if (SaveGameInstance)
        {
            // Optionally, you can store this in a property if you want to access it later
            HighScore = SaveGameInstance->HighScore;
        }
        else
        {
            HighScore = 0.0f;
        }
    }
}

/**
 *  Loads the high score from the specified save slot, compares it with the current score,
 * and saves the current score if it's higher.
 * 
 * This function is part of the persistence layer for the points allocation system.
 * The points earned during activities are saved here and can later be redeemed for rewards:
 * - 100 Points: Avatar customization options
 * - 200 Points: Exclusive guided meditation sessions
 * - 300 Points: Discounts on premium content
 * - 500 Points: Special self-care locations or activities
 * 
 * @param CurrentScore The score (elapsed time) achieved in the current session.
 */
void USecondCounterComponent::CheckAndSaveHighScore(float CurrentScore)
{
    // Ensure SaveSlotName is not empty before attempting to load/save
    if (SaveSlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SecondCounterComponent: SaveSlotName is empty. Cannot save high score."));
        return;
    }

    // Attempt to load the existing save game object.
    UHighScoreSaveGame* SaveGameInstance = Cast<UHighScoreSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

    // If no save game exists, create a new instance.
    if (!SaveGameInstance)
    {
        SaveGameInstance = Cast<UHighScoreSaveGame>(UGameplayStatics::CreateSaveGameObject(UHighScoreSaveGame::StaticClass()));
        // Check if creation was successful
        if (!SaveGameInstance)
        {
             UE_LOG(LogTemp, Error, TEXT("SecondCounterComponent: Failed to create SaveGameObject of type UHighScoreSaveGame."));
             return; // Cannot proceed without a save game instance
        }
        // Initialize high score in the new save instance if needed (optional, defaults to 0)
        // SaveGameInstance->HighScore = 0.0f;
    }

    // Compare the current score with the loaded/initialized high score.
    if (CurrentScore > SaveGameInstance->HighScore)
    {
        // Update the high score in the save game object.
        SaveGameInstance->HighScore = CurrentScore;
        // Save the updated object back to the slot.
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, UserIndex);
        // Optional: Log that a new high score was saved.
        // UE_LOG(LogTemp, Log, TEXT("New high score saved for %s: %.2f"), *SaveSlotName, CurrentScore);
    }
}

/**
 *  Called every frame. If the counter is running (bIsCounting is true),
 * it adds the frame's delta time to ElapsedTime and updates the associated ScoreWidget.
 * 
 * This tracking function is essential for timing activities that award points based on duration:
 * - Mindfulness Meditation: 10 points for 15-minute sessions
 * - Deep Breathing: 3 points for 5-minute sessions
 * - Journaling: 5 points for 10-minute sessions
 * - Stretching: 4 points for 10-minute routines
 * - Short Walk: 5 points for 15-minute walks
 * - Workout Session: 10 points for 30-minute workouts
 * - Creative Drawing: 6 points for 20-minute sessions
 * 
 * @param DeltaTime Time elapsed since the last frame.
 * @param TickType The type of tick.
 * @param ThisTickFunction Internal tick function structure.
 */
void USecondCounterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCounting)
    {
        ElapsedTime += DeltaTime;

        // Update the timer widget every tick
        if (CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
        {
            CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->UpdateActivityTimer(ElapsedTime, TargetTime, true, TEXT("Time"));
        }

        

        if (IsActivityComplete() && bIsCounting)
        {
            StopCounter();
        }
    }
}

/**
 *  Starts the counter.
 * Sets the counting flag to true and makes the associated score widget visible.
 * Used to begin tracking time for activities that award points based on duration.
 */
void USecondCounterComponent::StartCounter()
{
    if (CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
    {

        CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->UpdateActivityTimer(0.0f, TargetTime, true, TEXT("Time"));
        CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->SetVisibility(ESlateVisibility::Visible);
    }

    bIsCounting = true;
    bHasStopped = false;
}

/**
 *  Stops the counter.
 * Sets the counting flag to false. Does not hide the widget or save the score.
 * Can be used to pause an activity without finalizing the session or awarding points.
 */
void USecondCounterComponent::StopCounter()
{
    if (bHasStopped) return;
    bHasStopped = true;
    bIsCounting = false;
    ClampElapsedTime();

    // Update the timer widget with the final value
    if (CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
    {
        CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->UpdateActivityTimer(ElapsedTime, TargetTime, true, TEXT("Time"));
        CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->SetVisibility(ESlateVisibility::Hidden);
    }

    // Always save the high score if appropriate
    CheckAndSaveHighScore(ElapsedTime);

    // Only award points if activity was completed
    if (CachedEscapeCharacter && ElapsedTime >= TargetTime)
    {
        CachedEscapeCharacter->AggregatedScore += CompletionPoints;
        PrintScoreDebugMessage(CachedEscapeCharacter, FString::Printf(TEXT("[StopCounter] AggregatedScore after adding points: %f"), CachedEscapeCharacter->AggregatedScore), FColor::Yellow);
        // Animate the aggregated score in the ScoreWidget
       
        // Save AggregatedScore to WellnessSaveGame
        const FString WellnessSaveSlot = TEXT("WellnessSaveData");
        const int32 WellnessUserIndex = 0;
        UWellnessSaveGame* SaveGameInstance = Cast<UWellnessSaveGame>(UGameplayStatics::LoadGameFromSlot(WellnessSaveSlot, WellnessUserIndex));
        if (!SaveGameInstance)
        {
            SaveGameInstance = Cast<UWellnessSaveGame>(UGameplayStatics::CreateSaveGameObject(UWellnessSaveGame::StaticClass()));
            PrintScoreDebugMessage(CachedEscapeCharacter, TEXT("[StopCounter] Created new WellnessSaveGame instance."), FColor::Magenta);
        }
        if (SaveGameInstance)
        {
            SaveGameInstance->AggregatedScore = CachedEscapeCharacter->AggregatedScore;
            bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, WellnessSaveSlot, WellnessUserIndex);
            PrintScoreDebugMessage(CachedEscapeCharacter, FString::Printf(TEXT("[StopCounter] Saved AggregatedScore: %f (Success: %s)"), CachedEscapeCharacter->AggregatedScore, bSaveSuccess ? TEXT("true") : TEXT("false")), bSaveSuccess ? FColor::Green : FColor::Red);
        }
    }
}

/**
 *  Resets the elapsed time to zero.
 * Does not affect the counting state or widget visibility.
 * Useful when restarting an activity or abandoning a session without awarding points.
 */
void USecondCounterComponent::ResetCounter()
{
    ElapsedTime = 0.0f;
    bHasStopped = false;

    // Reset the timer widget
    if (CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget())
    {
        CachedEscapeCharacter->GetActivityUIWidget()->GetTimerWidget()->UpdateActivityTimer(0.0f, TargetTime, true, TEXT("Time"));
    }

}

/**
 *  Returns the progress towards the target time as a percentage (0.0 to 1.0).
 *  @return The ratio of ElapsedTime to TargetTime, clamped between 0 and 1.
 */
float USecondCounterComponent::GetProgressPercentage() const
{
    if (TargetTime <= 0.0f)
    {
        return 1.0f; // Avoid division by zero
    }
    
    // Calculate progress and clamp between 0 and 1
    return FMath::Clamp(ElapsedTime / TargetTime, 0.0f, 1.0f);
}

/**
 *  Returns the time remaining until the target time is reached.
 *  @return The difference between TargetTime and ElapsedTime, or 0 if target is reached.
 */
float USecondCounterComponent::GetTimeRemaining() const
{
    // Return the remaining time, or 0 if the target time has been reached or exceeded
    return FMath::Max(0.0f, TargetTime - ElapsedTime);
}

void USecondCounterComponent::UpdateElapsedTime(float ElapsedTimeP)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Timer Updated2"));

    ElapsedTime += ElapsedTimeP;
    ClampElapsedTime();
    if (TimerWidget)
    {
        TimerWidget->UpdateActivityTimer(ElapsedTime, TargetTime, true);
    }
}

// Private helper to clamp ElapsedTime to [0, TargetTime]
void USecondCounterComponent::ClampElapsedTime()
{
    if (ElapsedTime >= TargetTime)
    {
        ElapsedTime = TargetTime;
    }
}
