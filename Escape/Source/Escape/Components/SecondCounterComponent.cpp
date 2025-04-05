#include "SecondCounterComponent.h"
#include "../SaveGame/HighScoreSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include <Escape/EscapeCharacter.h>

USecondCounterComponent::USecondCounterComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsCounting = false;
    ElapsedTime = 0.0f;
}

void USecondCounterComponent::BeginPlay()
{
    Super::BeginPlay();
    OwningCharacter = Cast<ACharacter>(GetOwner());



}

void USecondCounterComponent::StopAndSaveScore()
{

    StopCounter();
    CheckAndSaveHighScore(GetElapsedTime());

}

void USecondCounterComponent::CheckAndSaveHighScore(float CurrentScore)
{
    // Load the saved high score from the slot
    UHighScoreSaveGame* SaveGameInstance = Cast<UHighScoreSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
    if (!SaveGameInstance)
    {
        // If no save exists, create a new one.
        SaveGameInstance = Cast<UHighScoreSaveGame>(UGameplayStatics::CreateSaveGameObject(UHighScoreSaveGame::StaticClass()));
    }

    // Check if the current score exceeds the stored high score
    if (CurrentScore > SaveGameInstance->HighScore)
    {
        SaveGameInstance->HighScore = CurrentScore;
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, UserIndex);
    }
    ScoreWidget->SetVisibility(ESlateVisibility::Collapsed);

}

void USecondCounterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCounting)
    {
        // Add delta time add it to our elapsed time.
        ElapsedTime += DeltaTime;
		ScoreWidget->UpdateScore(ElapsedTime);
    }
}

void USecondCounterComponent::StartCounter()
{

    ScoreWidget->SetVisibility(ESlateVisibility::Visible);
    bIsCounting = true;
}



void USecondCounterComponent::StopCounter()
{
    bIsCounting = false;
}

void USecondCounterComponent::ResetCounter()
{
    ElapsedTime = 0.0f;

}
