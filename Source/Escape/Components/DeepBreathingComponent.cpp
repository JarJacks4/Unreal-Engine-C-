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
   OwningCharacter = Cast<ACharacter>(GetOwner());   
   DeepBreathingMusicComponent = nullptr; // Initialized as null
   DeepBreathingDuration = 0.0f; // Default duration is 0 (toggle mode)
   DeepBreathingTimer = 0.0f; // Initialize timer counter
   CachedEscapeCharacter = Cast<AEscapeCharacter>(OwningCharacter.Get());
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
       if (DeepBreathingDuration > 0.0f)
       {
           DeepBreathingTimer += DeltaTime;
       }
   }  
}  

/**
 *  Starts the deep breathing exercise.
 * Displays the breathing bubble widget, plays the music, and initiates the inhale/exhale cycle based on BreathingType.
 */
void UDeepBreathingComponent::StartDeepBreathing()
{
    if (!CachedEscapeCharacter) return;
    UActivityUIWidget* ActivityWidget = CachedEscapeCharacter->GetActivityUIWidget();
    if (!ActivityWidget) return;
    ActivityWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    UBreathingBubbleWidget* BreathingWidget = ActivityWidget->GetDeepBreathingWidget();
    if (!BreathingWidget) return;

    int32 CurrentCompletionPoints = DefaultCompletionPoints;
    float CurrentBreathingDuration = DeepBreathingDuration;
    USoundCue* MusicToPlay = DeepBreathingMusic;
    float InitialBubbleScale = 1.5f;
    EDeepBreathingStatus InitialStatus = EDeepBreathingStatus::BreathingIn;
    ScaleDirection = -1.0f;

    switch (BreathingType)
    {
        case EBreathingType::Deep:
            CurrentCompletionPoints = 3;
            CurrentBreathingDuration = 300.0f;
            UE_LOG(LogTemp, Log, TEXT("Starting Deep Breathing (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::Basic:
            CurrentCompletionPoints = 10;
            UE_LOG(LogTemp, Log, TEXT("Starting Basic Breathing (%d points, %.1f seconds)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::BoxBreathing:
            UE_LOG(LogTemp, Log, TEXT("Starting Box Breathing (%d points, %.1f seconds) (Not fully implemented)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        case EBreathingType::FourSevenEight:
            UE_LOG(LogTemp, Log, TEXT("Starting 4-7-8 Breathing (%d points, %.1f seconds) (Not fully implemented)"), CurrentCompletionPoints, CurrentBreathingDuration);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown Breathing Type selected!"));
            break;
    }

    if (MusicToPlay && GetWorld())
    {
        if (!DeepBreathingMusicComponent)
        {
            DeepBreathingMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), MusicToPlay);
            if (DeepBreathingMusicComponent)
            {
                DeepBreathingMusicComponent->Play();
            }
        }
    }

    BubbleScale = InitialBubbleScale;
    BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale));
    BreathingWidget->BubbleButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ActivityWidget->GetTapButton()->SetVisibility(ESlateVisibility::Visible);
    BreathingWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    ActivityWidget->GetTargetBubble()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    if (CachedEscapeCharacter->SecondCounterComponent)
    {
        CachedEscapeCharacter->SecondCounterComponent->CompletionPoints = CurrentCompletionPoints;
        CachedEscapeCharacter->SecondCounterComponent->TargetTime = CurrentBreathingDuration;
        CachedEscapeCharacter->SecondCounterComponent->ResetCounter();
        CachedEscapeCharacter->SecondCounterComponent->StartCounter();
        if (UScoreWidget* ScoreWidget = CachedEscapeCharacter->SecondCounterComponent->GetScoreWidget())
        {
            ScoreWidget->UpdateActivityProgress(0.0f, CurrentBreathingDuration, CurrentCompletionPoints);
        }
    }

    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::DeepBreathing);
    SetDeepBreathingStatus(InitialStatus);

    ClearDeepBreathingTimer();
    DeepBreathingTimer = 0.0f;
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

    CachedEscapeCharacter->SetMinuteGoalActionState(EMinuteGoalActionsState::Idle);
    CachedEscapeCharacter->DeepBreathingComponent->SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
    BreathingWidget->SetVisibility(ESlateVisibility::Hidden);
    CachedEscapeCharacter->GetActivityUIWidget()->GetTapButton()->SetVisibility(ESlateVisibility::Hidden);
    CachedEscapeCharacter->GetActivityUIWidget()->GetTargetBubble()->SetVisibility(ESlateVisibility::Hidden);

    if (BreathingWidget)  
    {  
        if (DeepBreathingMusicComponent)
        {
            DeepBreathingMusicComponent->Stop();
        }
        CachedEscapeCharacter->SecondCounterComponent->StopCounter();
    }  
    ClearDeepBreathingTimer();
}  

/**
 *  Called when the player taps the breathing bubble.
 * Determines if the tap was within the correct timing window and adjusts the exercise accordingly.
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
        case EBreathingType::Basic:
            if (DeepBreathingStatus == EDeepBreathingStatus::BreathingIn && BubbleScale >= 0.4f && BubbleScale <= 0.8f)
            {
                bPerfectTap = true;
                BubbleScale = 0.5f;
                ScaleDirection = 1.0f;
                SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
                UE_LOG(LogTemp, Log, TEXT("Breathing: Good Tap!"));
            }
            break;
        case EBreathingType::BoxBreathing:
            UE_LOG(LogTemp, Log, TEXT("Box Breathing: Tap registered"));
            break;
        case EBreathingType::FourSevenEight:
            UE_LOG(LogTemp, Log, TEXT("4-7-8 Breathing: Tap registered"));
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
        { CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(-0.5); }
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
        case EBreathingType::Basic:
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
                 if (CachedEscapeCharacter->SecondCounterComponent) { CachedEscapeCharacter->SecondCounterComponent->UpdateElapsedTime(-1); }
                 ScaleDirection = 1.0f;
                 SetDeepBreathingStatus(EDeepBreathingStatus::BreathingOut);
            }
            break;

        case EBreathingType::BoxBreathing:
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            break;

        case EBreathingType::FourSevenEight:
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            break;

        default:
             BubbleScale += ScaleDirection * DeltaTime * 0.5f;
             if (BubbleScale >= 1.5f) ScaleDirection = -1.0f;
             else if (BubbleScale < 0.5f) ScaleDirection = 1.0f;
            break;
    }

    BreathingWidget->SetRenderScale(FVector2D(BubbleScale, BubbleScale));  
}  

/**
 *  Sets the deep breathing duration in seconds.
 *  @param Duration The new duration in seconds (0 for toggle mode)
 */
void UDeepBreathingComponent::SetDeepBreathingDuration(float Duration)
{
    DeepBreathingDuration = FMath::Max(Duration, 0.0f);
    if (GetWorld() && OwningCharacter.IsValid())
    {
        if (!CachedEscapeCharacter) CachedEscapeCharacter = Cast<AEscapeCharacter>(OwningCharacter.Get());
        AEscapeCharacter* PlayerCharacter = CachedEscapeCharacter;
        if (PlayerCharacter && PlayerCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
        {
            GetWorld()->GetTimerManager().ClearTimer(DeepBreathingTimerHandle);
            if (DeepBreathingDuration > 0.0f)
            {
                float RemainingTime = FMath::Max(DeepBreathingDuration - DeepBreathingTimer, 0.1f);
                GetWorld()->GetTimerManager().SetTimer(
                    DeepBreathingTimerHandle,
                    this,
                    &UDeepBreathingComponent::OnDeepBreathingComplete,
                    RemainingTime,
                    false
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
    DeepBreathingTimer = 0.0f;
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
    if (!OwningCharacter.IsValid()) return;
    if (!CachedEscapeCharacter) CachedEscapeCharacter = Cast<AEscapeCharacter>(OwningCharacter.Get());
    AEscapeCharacter* PlayerCharacter = CachedEscapeCharacter;
    if (!PlayerCharacter) return;
    if (PlayerCharacter->GetMinuteGoalActionsState() == EMinuteGoalActionsState::DeepBreathing)
    {
        ApplyDeepBreathingEffects();
    }
}

/**
 *  Applies effects when deep breathing completes naturally (timer-based only).
 * Example: add calm effect or stress reduction based on BreathingType.
 */
void UDeepBreathingComponent::ApplyDeepBreathingEffects()
{
    if (OwningCharacter.IsValid())
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
    }
}
