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
    UMeditationComponent();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartMeditation();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StopMeditation();

    UFUNCTION(BlueprintCallable)
    void SetBlockRef(AWellnessBlock* GottenBlock) { BlockRef = GottenBlock; };
    
    UFUNCTION(BlueprintCallable)
    AWellnessBlock* GetBlockRef() { return BlockRef; };
    
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationDuration(float Duration);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    float GetMeditationDuration() const { return MeditationDuration; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    float GetMeditationTimer() const { return MeditationTimer; }
    
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ClearMeditationTimer();
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Meditation")
    bool IsMeditationTimerActive() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    int32 DefaultCompletionPoints = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    EMeditationType MeditationType = EMeditationType::Guided;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<AWellnessBlock> BlockRef;

    UPROPERTY(EditAnywhere, Category = "Meditation", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MeditationDuration = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> GuidedMeditationMusic;

    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MindfulnessMeditationMusic;

    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> CosmicMeditationMusic;

    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MeditationPadMusic;

    UPROPERTY(EditAnywhere, Category = "Meditation|Audio")
    TObjectPtr<USoundCue> MeditationMusic;

    UPROPERTY(VisibleInstanceOnly, Category = "Meditation", Transient)
    float MeditationTimer = 0.0f;

    FTimerHandle MeditationTimerHandle;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> MeditationMusicComponent;

    void HandleMeditationStart();
    void HandleMeditationStop();
    void ApplyMeditationEffects();
    void OnMeditationComplete();
    AEscapeCharacter* CachedEscapeCharacter = nullptr;
};
