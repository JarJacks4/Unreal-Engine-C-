// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"

#include "ILipSyncProcessor.h"
#include "DSP/FFTAlgorithm.h"
#include "UObject/Object.h"
#include "Tasks/Pipe.h"
#include "AudioResampler.h"
#include "SampleBuffer.h"
#include "RuntimeVisemeGenerator.generated.h"

/**
 * Viseme generator that can generate visemes from audio data
 */
UCLASS(BlueprintType)
class RUNTIMEMETAHUMANLIPSYNC_API URuntimeVisemeGenerator : public UObject
{
	GENERATED_BODY()
	
public:
	URuntimeVisemeGenerator();

	virtual void BeginDestroy() override;

	/**
	 * Create a new instance of the runtime viseme generator
	 *
	 * @return Created runtime viseme generator
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Viseme Generator")
	static URuntimeVisemeGenerator* CreateRuntimeVisemeGenerator();

	/**
	 * Processes raw PCM audio data to generate viseme weights
	 *
	 * @param PCMData The raw PCM audio data to process
	 * @param SampleRate The sample rate of the audio data
	 * @param NumOfChannels The number of channels in the audio data
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Viseme Generator")
	void ProcessAudioData(TArray<float> PCMData, int32 SampleRate, int32 NumOfChannels);

	/**
	 * Retrieves the current viseme weights
	 *
	 * @return An array of viseme weights representing the current state of the visemes
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Viseme Generator")
	TArray<float> GetVisemeWeights() const;

	/**
	 * Retrieves the names of the visemes supported by the system
	 *
	 * @return An array of strings containing the names of the visemes
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Viseme Generator")
	static TArray<FString> GetVisemeNames();

	void SetVisemeWeights(TArray<float> InWeights);

	float GetLaughterScore() const { return LaughterScore; }

private:

	/** The laughter score calculated from the audio data */
	float LaughterScore = 0;

	/** The current viseme weights calculated from the audio data */
	TArray<float> Visemes;

	/** Data guard (mutex) for thread safety */
	mutable TSharedPtr<FCriticalSection> DataGuard;

	/** The size of the buffer used for accumulating PCM data */
	int32 BufferSize = 4096;

	/** Accumulated PCM data for processing */
	TArray<float> AccumulatedPCMData;

	/** Task pipe for handling audio processing tasks */
	TUniquePtr<UE::Tasks::FPipe> AudioTaskPipe;

	ILipSyncProcessor* LipSyncProcessor = nullptr;

	/**
	 * Mixing RAW Data to a different number of channels
	 *
	 * @param RAWData RAW data for mixing
	 * @param SampleRate Sample rate of the RAW data
	 * @param SourceNumOfChannels Source number of channels in the RAW data
	 * @param DestinationNumOfChannels Destination number of channels in the RAW data
	 * @param RemixedRAWData Remixed RAW data
	 * @return True if the RAW data was successfully mixed
	 */
	static bool MixChannelsRAWData(Audio::FAlignedFloatBuffer& RAWData, int32 SampleRate, int32 SourceNumOfChannels, int32 DestinationNumOfChannels, Audio::FAlignedFloatBuffer& RemixedRAWData);

private:
	bool LipSyncProcessorInitialized = false;
};