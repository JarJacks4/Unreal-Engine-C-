// TTSHelper.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "TTSHelper.generated.h"

class ULipSyncDriver;

UCLASS()
class ESCAPE_API UTTSHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Synthesizes text with Azure TTS and plays it on the given AudioComponent.
     * @param AzureKey   - Your Speech key
     * @param AzureRegion- Region, e.g. "eastus"
     * @param VoiceName  - e.g. "en-US-JennyNeural"
     * @param Text       - The text to speak
     * @param AudioComp  - Target AudioComponent (e.g., on your MetaHuman)
     */
    UFUNCTION(BlueprintCallable, Category = "TTS")
    static void SynthesizeAndPlay_Azure(const FString& AzureKey,
        const FString& AzureRegion,
        const FString& VoiceName,
        const FString& Text,
        UAudioComponent* AudioComp);
};

