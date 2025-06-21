// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ChatAudioTextHolder.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeechToTextResult, const FString&, Result);

UCLASS()
class ESCAPE_API AChatAudioTextHolder : public AActor
{
    GENERATED_BODY()
    public:


        /** Event triggered when speech-to-text result is received */
        UPROPERTY(BlueprintAssignable, Category = "Speech|Android")
        FOnSpeechToTextResult OnSpeechToTextResultEvent;

        /** Event triggered when speech-to-text result is received */
        UPROPERTY(BlueprintReadOnly, Category = "Speech|Android")
        FString OnSpeechToTextResultEventText;

        void IWSetText(FString Result) { OnSpeechToTextResultEventText = Result; OnSpeechToTextResultEvent.Broadcast(OnSpeechToTextResultEventText); };
	
};
