#include "DeepBreathingComponent.h"  
#include "../Widgets/BreathingBubbleWidget.h" 
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/Widget.h"
#include "Components/Button.h"
#include "../EscapeCharacter.h"
#include "TimerManager.h"

/**
 *  Constructor for UDeepBreathingComponent.
 * Initializes default values for the component.
 */
UDeepBreathingComponent::UDeepBreathingComponent()  
{  
   PrimaryComponentTick.bCanEverTick = true; // Enable ticking for scale updates  
   BubbleScale = 1.5f; // Initial scale  
   ScaleDirection = -1.0f; // Start shrinking  
   // Cache the owning character for efficiency  
   DeepBreathingMusicComponent = nullptr; // Initialized as null
   DeepBreathingDuration = 0.0f; // Default duration is 0 (toggle mode)
   CachedEscapeCharacter = Cast<AEscapeCharacter>(GetOwner());
}  

/**
 *  Called every frame if the component is active.
 * Updates the breathing bubble's scale when the exercise is active.
 *  DeltaTime Time since the last frame.
 *  TickType Type of tick (e.g., game tick).
 *  ThisTickFunction The function being ticked.
 */
void UDeepBreathingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)  
{  
   Super::TickComponent(DeltaTime, TickType, ThisTickFunction);  
   if (CachedEscapeCharacter && CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
   {
       UpdateBreathingBubble(DeltaTime);
   }
}  

/**
 *  Starts the deep breathing exercise.
 * Displays the breathing bubble widget, plays the music, and initiates the inhale/exhale cycle based on BreathingType.
 */
void UDeepBreathingComponent::StartDeepBreathing()
{
    if (!CachedEscapeCharacter) { /* Cache or return */ return; }
    UActivityUIWidget* ActivityWidget = CachedEscapeCharacter->GetActivityUIWidget();
    if (!ActivityWidget) return;
    ActivityWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    UBreathingBubbleWidget* BreathingWidget = ActivityWidget->GetDeepBreathingWidget();
    if (!BreathingWidget) return;
    // --- Breathing Type Specific Setup ---
    int32 CurrentCompletionPoints = 0;
    int32 CurrentMinimumPoints = 0;
    float CurrentBreathingDuration = DeepBreathingDuration;
    DeepBreathingMusicComponent = UGameplayStatics::CreateSound2D(GetWorld(), DeepBreathingMusic);
    DeepBreathingMusicComponent->Play();
    float InitialBubbleScale = 1.5f;
    EDeepBreathingStatus InitialStatus = EDeepBreathingStatus::BreathingIn;
    bool bShowTargetBubble = true;
    ScaleDirection = -1.0f; // Default start shrinking (inhale visual)

    switch (BreathingType)
    {
        case EBreathingType::Deep: // General Deep Breathing
            CurrentCompletionPoints = CompletionPoints_Deep;
            CurrentMinimumPoints = MinimumPoints_Deep;
            CurrentBreathingDuration = DeepBreathingDuration;
            UE_LOG(LogTemp, Log, TEXT("Starting Deep Breathing (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::Basic:
            CurrentCompletionPoints = CompletionPoints_Basic;
            CurrentMinimumPoints = MinimumPoints_Basic;
            CurrentBreathingDuration = DeepBreathingDuration;
            UE_LOG(LogTemp, Log, TEXT("Starting Basic Breathing (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::BoxBreathing:
            CurrentCompletionPoints = CompletionPoints_Box;
            CurrentMinimumPoints = MinimumPoints_Box;
            CurrentBreathingDuration = DeepBreathingDuration;
            UE_LOG(LogTemp, Log, TEXT("Starting Box Breathing (%d points, %.1f seconds) (Not fully implemented)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::FourSevenEight:
            CurrentCompletionPoints = CompletionPoints_478;
            CurrentMinimumPoints = MinimumPoints_478;
            CurrentBreathingDuration = DeepBreathingDuration;
            UE_LOG(LogTemp, Log, TEXT("Starting 4-7-8 Breathing (%d points, %.1f seconds) (Not fully implemented)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown Breathing Type selected!"));
            break;
    }



    // Setup UI Elements
    BubbleScale = InitialBubbleScale;
    BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale));
    BreathingWidget->BubbleButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ActivityWidget->GetTapButton()->SetVisibility(ESlateVisibility::Visible);
    BreathingWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    ActivityWidget->GetTargetBubble()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);


    // Setup time Counter
    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CurrentCompletionPoints;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = CurrentBreathingDuration;
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();

    }

    // Set Initial State
    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::DeepBreathing);
    SetDeepBreathingStatus(InitialStatus);

    if (CurrentBreathingDuration > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DeepBreathingTimerHandle,
            this,
            &UDeepBreathingComponent::OnDeepBreathingComplete,
            CurrentBreathingDuration,
            false
        );
    }
}

/**
 *  Stops the deep breathing exercise.
 * Removes the breathing bubble widget, stops the music, and resets the state.
 */
void UDeepBreathingComponent::StopDeepBreathing()  
{  
    if (!CachedEscapeCharacter) return;
    UBreathingBubbleWidget* BreathingWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetDeepBreathingWidget();

   // Ensure the state changes to idle when stopping
   CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);
   CachedEscapeCharacter->DeepBreathingComponent->SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
   BreathingWidget->SetVisibility(ESlateVisibility::Hidden);
   CachedEscapeCharacter->GetActivityUIWidget()->GetTapButton()->SetVisibility(ESlateVisibility::Hidden);
   CachedEscapeCharacter->GetActivityUIWidget()->GetTargetBubble()->SetVisibility(ESlateVisibility::Hidden);
   {  
       if (DeepBreathingMusicComponent)
       {
           DeepBreathingMusicComponent->Stop();
       }
       UButton* TapButton = CachedEscapeCharacter->GetActivityUIWidget()->GetTapButton();

       CachedEscapeCharacter->SecondCounterComponent->StopCounter();
   }  

   // Clear the timer
   ClearDeepBreathingTimer();
   ApplyDeepBreathingEffects();
}  

/**
 *  Called when the player taps the breathing bubble.
 * Determines if the tap was within the correct timing window and adjusts the exercise accordingly.
 * Logic might need adjustment based on BreathingType.
 */
void UDeepBreathingComponent::OnBreathingBubbleTapped()
{  
    if (!CachedEscapeCharacter) return;
    UBreathingBubbleWidget* BreathingWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetDeepBreathingWidget();
    if (!BreathingWidget) return;

    bool bPerfectTap = false;
    switch (BreathingType)
    {
        case EBreathingType::Deep:
        case EBreathingType::Basic: // Basic uses the same tap mechanic for now
            if (DeepBreathingStatus == EDeepBreathingStatus::BreathingIn && BubbleScale >= 0.4f && BubbleScale <= 0.8f)
            {
                bPerfectTap = true;
                BubbleScale = 0.5f; // Reset to min scale before expanding
                ScaleDirection = 1.0f; // Start expanding (exhale visual)
                SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
                UE_LOG(LogTemp, Log, TEXT("Breathing: Good Tap!"));
            }
            break;
        case EBreathingType::BoxBreathing:
            UE_LOG(LogTemp, Log, TEXT("Box Breathing: Tap registered (No specific logic yet)"));
            // No tap mechanic defined yet
            break;
        case EBreathingType::FourSevenEight:
            UE_LOG(LogTemp, Log, TEXT("4-7-8 Breathing: Tap registered (No specific logic yet)"));
            // No tap mechanic defined yet
            break;
        default:
            break;
    }

    if (bPerfectTap)
    {
         BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale));
    }
    else if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
    {
        UE_LOG(LogTemp, Warning, TEXT("Breathing: Bad Tap or incorrect state/timing!"));
        if (CachedEscapeCharacter->SecondCounterComponent)
        {
            return;
        }
    }
}  

/**
 *  Updates the breathing bubble's scale based on the current state, time, and BreathingType.
 *  DeltaTime Time since the last frame.
 */
void UDeepBreathingComponent::UpdateBreathingBubble(float DeltaTime)  
{  
    if (!CachedEscapeCharacter || !CachedEscapeCharacter->GetActivityUIWidget()) return;
    UBreathingBubbleWidget* BreathingWidget = CachedEscapeCharacter->GetActivityUIWidget()->GetDeepBreathingWidget();
    if (!BreathingWidget) return;

    switch (BreathingType)
    {
        case EBreathingType::Deep:
        case EBreathingType::Basic: // Basic uses the same scaling mechanic for now
            BubbleScale += ScaleDirection * DeltaTime * 0.5f;
            if (BubbleScale >= 1.5f)
            {
                BubbleScale = 1.5f;
                ScaleDirection = -1.0f;
                SetDeepBreathingStatus(EDeepBreathingStatus::BreathingIn);
            }
            else if (BubbleScale <= 0.5f)
            {
                 BubbleScale = 0.5f;
                 // Penalty for letting it shrink fully without tapping
                 ScaleDirection = 1.0f;
                 SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
            }
            break;

        case EBreathingType::BoxBreathing:
            // Placeholder: Use basic logic
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            // TODO: Implement Box Breathing specific scaling (4 phases)
            break;

        case EBreathingType::FourSevenEight:
            // Placeholder: Use basic logic
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            // TODO: Implement 4-7-8 specific scaling (timed phases)
            break;

        default:
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            break;
    }
    
    BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale)); // Shrink  
}  

/**
 *  Sets the deep breathing duration in seconds.
 *  @param Duration The new duration in seconds (0 for toggle mode)
 */
void UDeepBreathingComponent::SetDeepBreathingDuration(float Duration)
{
    // Ensure duration is non-negative
    DeepBreathingDuration = FMath::Max(Duration, 0.0f);
    
    // If deep breathing is already in progress and timer is active, update the timer
    if (GetWorld() && CachedEscapeCharacter)
    {
        if (!CachedEscapeCharacter) 
        if (CachedEscapeCharacter && CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
        {
            // Clear existing timer
            GetWorld()->GetTimerManager().ClearTimer(DeepBreathingTimerHandle);
            
            // If new duration is > 0, set a new timer with remaining time
            if (DeepBreathingDuration > 0.0f)
            {
                // Calculate remaining time based on new duration and already elapsed time
                
                GetWorld()->GetTimerManager().SetTimer(
                    DeepBreathingTimerHandle,
                    this,
                    &UDeepBreathingComponent::OnDeepBreathingComplete,
                    DeepBreathingDuration,
                    false // Single-shot timer
                );
            }
        }
    }
}

/**
 *  Clears the deep breathing timer, resetting elapsed time to zero.
 *  Useful when stopping the exercise prematurely.
 */
void UDeepBreathingComponent::ClearDeepBreathingTimer()
{
    
    // Clear the timer if it's active
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DeepBreathingTimerHandle);
    }
}

/**
 *  Checks if the deep breathing timer is currently active.
 *  @return True if deep breathing timer is running
 */
bool UDeepBreathingComponent::IsDeepBreathingTimerActive() const
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DeepBreathingTimerHandle);
}

/**
 *  Called when the timer completes (if `DeepBreathingDuration` > 0).
 * Applies effects and stops the exercise.
 */
void UDeepBreathingComponent::OnDeepBreathingComplete()
{
    // Ensure EscapeCharacter is valid before proceeding
    if (!CachedEscapeCharacter) return;

    // Only proceed if still doing deep breathing (guards against race conditions or manual stops)
    if (CachedEscapeCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
    {
		StopDeepBreathing(); // Stop the exercise

    }
}

/**
 *  Applies effects when deep breathing completes naturally (timer-based only).
 * Example: add calm effect or stress reduction based on BreathingType.
 */
void UDeepBreathingComponent::ApplyDeepBreathingEffects()
{
    if (CachedEscapeCharacter)
    {
        switch (BreathingType)
        {
            case EBreathingType::Deep:
                 UE_LOG(LogTemp, Log, TEXT("Deep Breathing Completed"));
                break;
            case EBreathingType::Basic:
                 UE_LOG(LogTemp, Log, TEXT("Basic Breathing Completed"));
                break;
            case EBreathingType::BoxBreathing:
                 UE_LOG(LogTemp, Log, TEXT("Box Breathing Completed"));
                break;
            case EBreathingType::FourSevenEight:
                 UE_LOG(LogTemp, Log, TEXT("4-7-8 Breathing Completed"));
                break;
            default:
                 UE_LOG(LogTemp, Log, TEXT("Deep Breathing Completed (Unknown Type)"));
                break;
        }
        // Add general gameplay logic here
    }
}
