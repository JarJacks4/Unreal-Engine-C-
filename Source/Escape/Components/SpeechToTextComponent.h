#pragma once

#include "CoreMinimal.h"
#include "../ChatAudioTextHolder.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "SpeechToTextComponent.generated.h"



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPE_API USpeechToTextComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION()
    virtual void BeginPlay() override;
    /**
    * Starts Android speech-to-text recognition. Only works on Android devices.
    */
    UFUNCTION(BlueprintCallable, Category = "Speech|Android")
    void StartSpeechToText();

    /**
    * Stops speech-to-text recognition. Works on both Android and iOS devices.
    */
    UFUNCTION(BlueprintCallable, Category = "Speech|Cross Platform")
    void StopSpeechToText();

    /**
    * Called from Java via JNI when speech recognition result is available.
    */
    UFUNCTION(BlueprintCallable, Category = "Speech|Android")
    static void OnSpeechToTextResult(FString Result);

    /**
    * Hold the text result of the speech recognition.
    */
    UPROPERTY(BlueprintReadOnly, Category = "Speech|Android")
    AChatAudioTextHolder* AudioTextHolder;
};

