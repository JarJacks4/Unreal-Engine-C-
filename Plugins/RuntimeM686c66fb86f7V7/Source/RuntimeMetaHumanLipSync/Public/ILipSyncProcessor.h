// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Features/IModularFeature.h"

/**
 * Interface for LipSync processor implementations that can be registered as modular features
 */
class RUNTIMEMETAHUMANLIPSYNC_API ILipSyncProcessor : public IModularFeature
{
public:
	virtual ~ILipSyncProcessor() = default;

	/**
	 * Initialize the LipSync processor
	 * @param ContextOwner The object that owns this lip sync context (e.g., character, audio component)
	 * @param SampleRate The sample rate of the audio data
	 * @param BufferSize The buffer size to use for processing
	 */
	virtual void Initialize(UObject* ContextOwner, int32 SampleRate, int32 BufferSize) = 0;
    
	/**
	 * Set callback for async processing results
	 * @param ContextOwner The object that owns this lip sync context
	 * @param Callback Function to call when processing is complete
	 */
	virtual void SetAsyncCallback(UObject* ContextOwner, TFunction<void(const TArray<float>&, float)> Callback) = 0;
    
	/**
	 * Process audio frame asynchronously
	 * @param ContextOwner The object that owns this lip sync context
	 * @param PCMData Raw PCM audio data
	 * @param NumSamples Number of samples in the data
	 */
	virtual void ProcessFrameAsync(UObject* ContextOwner, const float* PCMData, int32 NumSamples) = 0;

	/**
	 * Clean up resources for a specific context when no longer needed
	 * @param ContextOwner The object that owns this lip sync context
	 */
	virtual void CleanupContext(UObject* ContextOwner) = 0;
};