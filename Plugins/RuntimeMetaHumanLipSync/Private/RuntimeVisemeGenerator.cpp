// Georgy Treshchev 2025.

#include "RuntimeVisemeGenerator.h"

#include "Misc/EngineVersionComparison.h"
#include "Features/IModularFeatures.h"
#include "RuntimeMetaHumanLipSyncModule.h"
#include "SampleBuffer.h"
#include "DSP/AudioFFT.h"
#include "Tasks/Pipe.h"

URuntimeVisemeGenerator::URuntimeVisemeGenerator()
	: DataGuard(MakeShared<FCriticalSection>())
{
	AudioTaskPipe = MakeUnique<UE::Tasks::FPipe>(*FString::Printf(TEXT("AudioTaskPipe_%s"), *GetName()));
	ensureMsgf(AudioTaskPipe, TEXT("AudioTaskPipe is not initialized. This will cause issues with audio data appending"));
}

void URuntimeVisemeGenerator::BeginDestroy()
{
	if (LipSyncProcessor)
	{
		LipSyncProcessor->CleanupContext(this);
		LipSyncProcessor = nullptr;
	}
	
	Super::BeginDestroy();
}

URuntimeVisemeGenerator* URuntimeVisemeGenerator::CreateRuntimeVisemeGenerator()
{
    return NewObject<URuntimeVisemeGenerator>();
}

void URuntimeVisemeGenerator::ProcessAudioData(TArray<float> PCMData, int32 SampleRate, int32 NumOfChannels)
{
	if (NumOfChannels <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to mix audio data because the number of channels is invalid (%d)"), NumOfChannels);
		return;
	}
	
	if (IsInGameThread())
	{
		AudioTaskPipe->Launch(AudioTaskPipe->GetDebugName(), [WeakThis = MakeWeakObjectPtr(this), PCMData = MoveTemp(PCMData), SampleRate, NumOfChannels]() mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ProcessAudioData(MoveTemp(PCMData), SampleRate, NumOfChannels);
			}
			else
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to mix audio data because the runtime viseme generator object has been destroyed"));
			}
		}, UE::Tasks::ETaskPriority::BackgroundHigh);
		return;
	}

	if (NumOfChannels > 1)
	{
		Audio::FAlignedFloatBuffer PCMData_AlignedFloatBuffer = Audio::FAlignedFloatBuffer(MoveTemp(PCMData));
		MixChannelsRAWData(PCMData_AlignedFloatBuffer, SampleRate, NumOfChannels, 1, PCMData_AlignedFloatBuffer);
		PCMData = MoveTemp(PCMData_AlignedFloatBuffer);
	}

	UE_LOG(LogRuntimeMetaHumanLipSync, VeryVerbose, TEXT("Processing audio data with %d sample rate and %d num of samples"), SampleRate, PCMData.Num());

	if (!LipSyncProcessor)
	{
		static const FName LipSyncFeatureName(TEXT("RuntimeLipSyncProcessor"));
#if UE_VERSION_NEWER_THAN(5, 1, 0)
	IModularFeatures::FScopedLockModularFeatureList ScopedLockModularFeatureList;
#endif
    
		if (IModularFeatures::Get().IsModularFeatureAvailable(LipSyncFeatureName))
		{
			// Get the first available implementation
			TArray<ILipSyncProcessor*> LipSyncProcessors = IModularFeatures::Get().GetModularFeatureImplementations<ILipSyncProcessor>(LipSyncFeatureName);
        
			if (LipSyncProcessors.Num() > 0)
			{
				// Use the first available processor
				LipSyncProcessor = LipSyncProcessors[0];
			}
			else
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Warning, TEXT("LipSync feature is available but no implementations were found"));
				return;
			}
		}
		else
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Warning, TEXT("No LipSync processor feature available. Please refer to the documentation on how to enable it: <https://docs.georgy.dev/runtime-metahuman-lip-sync/how-to-use-the-plugin>"));
			return;
		}
	}

	if (!LipSyncProcessor)
	{
		return;
	}

	UObject* ContextOwner = this;

	// Initialize if needed
	if (!LipSyncProcessorInitialized)
	{
		LipSyncProcessor->Initialize(ContextOwner, SampleRate, BufferSize);
		LipSyncProcessor->SetAsyncCallback(ContextOwner, [this](const TArray<float>& NewVisemes, float NewLaughterScore)
		{
			FScopeLock Lock(&*DataGuard);
			Visemes = NewVisemes;
			LaughterScore = NewLaughterScore;
		});
		LipSyncProcessorInitialized = true;
	}

	// Process the audio data
	LipSyncProcessor->ProcessFrameAsync(ContextOwner, (const float*)PCMData.GetData(), PCMData.Num());
	
}

TArray<float> URuntimeVisemeGenerator::GetVisemeWeights() const
{
	FScopeLock Lock(&*DataGuard);
	return Visemes;
}

TArray<FString> URuntimeVisemeGenerator::GetVisemeNames()
{
	static TArray<FString> VisemeNames;
	if (VisemeNames.Num() == 0)
	{
		VisemeNames.Add(TEXT("sil"));
		VisemeNames.Add(TEXT("PP"));
		VisemeNames.Add(TEXT("FF"));
		VisemeNames.Add(TEXT("TH"));
		VisemeNames.Add(TEXT("DD"));
		VisemeNames.Add(TEXT("kk"));
		VisemeNames.Add(TEXT("CH"));
		VisemeNames.Add(TEXT("SS"));
		VisemeNames.Add(TEXT("nn"));
		VisemeNames.Add(TEXT("RR"));
		VisemeNames.Add(TEXT("aa"));
		VisemeNames.Add(TEXT("E"));
		VisemeNames.Add(TEXT("ih"));
		VisemeNames.Add(TEXT("oh"));
		VisemeNames.Add(TEXT("ou"));
	}
	return VisemeNames;
}

void URuntimeVisemeGenerator::SetVisemeWeights(TArray<float> InWeights)
{
	Visemes = MoveTemp(InWeights);
}

bool URuntimeVisemeGenerator::MixChannelsRAWData(Audio::FAlignedFloatBuffer& RAWData, int32 SampleRate, int32 SourceNumOfChannels, int32 DestinationNumOfChannels, Audio::FAlignedFloatBuffer& RemixedRAWData)
{
	if (SampleRate <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to mix audio data because the sample rate is invalid (%d)"), SampleRate);
		return false;
	}
	if (SourceNumOfChannels <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to mix audio data because the source number of channels is invalid (%d)"), SourceNumOfChannels);
		return false;
	}
	if (DestinationNumOfChannels <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to mix audio data because the destination number of channels is invalid (%d)"), DestinationNumOfChannels);
		return false;
	}

	// No need to mix if the number of channels are the same
	if (SourceNumOfChannels == DestinationNumOfChannels)
	{
		RemixedRAWData = MoveTemp(RAWData);
		return true;
	}

	Audio::TSampleBuffer<float> PCMSampleBuffer(RAWData, SourceNumOfChannels, SampleRate);
	{
		PCMSampleBuffer.MixBufferToChannels(DestinationNumOfChannels);
	}
	RemixedRAWData = Audio::FAlignedFloatBuffer(PCMSampleBuffer.GetData(), PCMSampleBuffer.GetNumSamples());
	return true;
}