// Georgy Treshchev 2025.

#include "RealisticMetaHumanLipSyncGenerator.h"

#include "RealisticMetaHumanLipSyncModel.h"
#include "RuntimeMetaHumanLipSyncModule.h"
#include "SampleBuffer.h"
#include "DSP/AudioFFT.h"
#include "Engine/AssetManager.h"
#include "Tasks/Pipe.h"
#include "AssetRegistry/AssetData.h"

URealisticMetaHumanLipSyncGenerator::URealisticMetaHumanLipSyncGenerator()
	: DataGuard(MakeShared<FCriticalSection>())
{
	AudioTaskPipe = MakeUnique<UE::Tasks::FPipe>(*FString::Printf(TEXT("AudioTaskPipe_%s"), *GetName()));
	ensureMsgf(AudioTaskPipe, TEXT("AudioTaskPipe is not initialized. This will cause issues with audio data appending"));

	// Initialize buffers
	AudioBuffer.SetNumZeroed(16000);
	CurveValues.SetNumZeroed(81);
	KalmanBuffer.SetNumZeroed(81 * 81);
}

URealisticMetaHumanLipSyncGenerator::~URealisticMetaHumanLipSyncGenerator()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	OrtSession.Reset();

	if (ModelDataPtr)
	{
		FMemory::Free(ModelDataPtr);
		ModelDataPtr = nullptr;
	}
#endif
}

URealisticMetaHumanLipSyncGenerator* URealisticMetaHumanLipSyncGenerator::CreateRealisticMetaHumanLipSyncGenerator(const FRealisticMetaHumanLipSyncConfig& Config)
{
	URealisticMetaHumanLipSyncGenerator* Generator = NewObject<URealisticMetaHumanLipSyncGenerator>();
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (!Generator->OrtEnvironment)
	{
		Generator->OrtEnvironment = MakeShared<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "MetaHumanLipSyncModel");
		Generator->GeneratorConfig = Config;
		Generator->InitializeModel();
	}
#endif
	return Generator;
}

void URealisticMetaHumanLipSyncGenerator::ProcessAudioData(TArray<float> PCMData, int32 SampleRate, int32 NumOfChannels)
{
	if (SampleRate <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to process audio data because the sample rate is invalid (%d)"), SampleRate);
		return;
	}

	if (NumOfChannels <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to process audio data because the number of channels is invalid (%d)"), NumOfChannels);
		return;
	}

	bool bNeedsRemixing = NumOfChannels > 1;
	bool bNeedsResampling = SampleRate != 16000;

	// Convert to mono if needed
	TArray<float> MonoData;
	if (bNeedsRemixing)
	{
		Audio::FAlignedFloatBuffer PCMData_AlignedFloatBuffer = Audio::FAlignedFloatBuffer(MoveTemp(PCMData));
		if (!MixChannelsRAWData(PCMData_AlignedFloatBuffer, SampleRate, NumOfChannels, 1, PCMData_AlignedFloatBuffer))
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Failed to mix audio data from %d to %d channels"), NumOfChannels, 1);
			return;
		}
		MonoData = MoveTemp(PCMData_AlignedFloatBuffer);
	}
	else
	{
		MonoData = MoveTemp(PCMData);
	}

	// Resample to 16kHz if needed
	TArray<float> ProcessedData;
	if (bNeedsResampling)
	{
		Audio::FAlignedFloatBuffer ProcessedData_AlignedFloatBuffer;
		Audio::FAlignedFloatBuffer MonoData_AlignedFloatBuffer = Audio::FAlignedFloatBuffer(MoveTemp(MonoData));
		if (!ResampleAudioData(MonoData_AlignedFloatBuffer, 1, SampleRate, 16000, ProcessedData_AlignedFloatBuffer))
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Failed to resample audio data from %d Hz to 16000 Hz"), SampleRate);
			return;
		}
		ProcessedData = MoveTemp(ProcessedData_AlignedFloatBuffer);
	}
	else
	{
		ProcessedData = MoveTemp(MonoData);
	}

	if (IsInGameThread())
	{
		AudioTaskPipe->Launch(AudioTaskPipe->GetDebugName(), [WeakThis = MakeWeakObjectPtr(this), ProcessedData = MoveTemp(ProcessedData)]() mutable
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ProcessAudioData(MoveTemp(ProcessedData), 16000, 1);
			}
			else
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to process audio data because the generator object has been destroyed"));
			}
		}, UE::Tasks::ETaskPriority::BackgroundHigh);
		return;
	}

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	// Always accumulate the incoming audio data (thread-safe)
	{
		FScopeLock Lock(&*DataGuard);
		AccumulatedAudioBuffer.Append(ProcessedData);

		// Prevent unbounded memory growth - remove oldest data if buffer gets too large
		if (AccumulatedAudioBuffer.Num() > MaxAccumulatedBufferSize)
		{
			const int32 ExcessSamples = AccumulatedAudioBuffer.Num() - MaxAccumulatedBufferSize;
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			AccumulatedAudioBuffer.RemoveAt(0, ExcessSamples, false);
#else
			AccumulatedAudioBuffer.RemoveAt(0, ExcessSamples, EAllowShrinking::No);
#endif
			UE_LOG(LogRuntimeMetaHumanLipSync, Warning, TEXT("Audio buffer overflow: discarded %d samples to prevent memory issues"), ExcessSamples);
		}
	}

	// Only schedule processing if the pipe is not already busy and we have enough data
	if (!bIsPipeProcessing.Load() && AccumulatedAudioBuffer.Num() >= ProcessingChunkSize)
	{
		bIsPipeProcessing.Store(true);
		
		AudioTaskPipe->Launch(AudioTaskPipe->GetDebugName(), [WeakThis = MakeWeakObjectPtr(this)]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ProcessAccumulatedAudio();
			}
			else
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to process accumulated audio because the generator object has been destroyed"));
			}
		}, UE::Tasks::ETaskPriority::BackgroundHigh);
	}
#endif
}

void URealisticMetaHumanLipSyncGenerator::ProcessAccumulatedAudio()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (!OrtSession.IsValid())
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Warning, TEXT("ONNX model not available, skipping audio processing"));
		bIsPipeProcessing.Store(false);
		return;
	}

	TArray<float> DataToProcess;
	
	// Extract data to process (thread-safe)
	{
		FScopeLock Lock(&*DataGuard);
		
		if (AccumulatedAudioBuffer.Num() >= ProcessingChunkSize)
		{
			// Take the largest chunk we can process efficiently
			const int32 ChunkSize = FMath::Min(AccumulatedAudioBuffer.Num(), ProcessingChunkSize * 4); // Process up to 4 chunks at once for efficiency
			DataToProcess.SetNumUninitialized(ChunkSize);
			FMemory::Memcpy(DataToProcess.GetData(), AccumulatedAudioBuffer.GetData(), ChunkSize * sizeof(float));

			// Remove processed data from the buffer
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			AccumulatedAudioBuffer.RemoveAt(0, ChunkSize, false);
#else
			AccumulatedAudioBuffer.RemoveAt(0, ChunkSize, EAllowShrinking::No);
#endif
		}
	}

	// Process the extracted data
	if (DataToProcess.Num() > 0)
	{
		// Process in chunks
		int32 ProcessedSamples = 0;
		while (ProcessedSamples + ProcessingChunkSize <= DataToProcess.Num())
		{
			TArray<float> ChunkToProcess;
			ChunkToProcess.SetNumUninitialized(ProcessingChunkSize);
			FMemory::Memcpy(ChunkToProcess.GetData(), DataToProcess.GetData() + ProcessedSamples, ProcessingChunkSize * sizeof(float));
			
			if (!ProcessFrameWithOnnx(ChunkToProcess))
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Warning, TEXT("Failed to process audio chunk with ONNX"));
				break;
			}
			
			ProcessedSamples += ProcessingChunkSize;
		}

		// If there are remaining samples that don't form a complete chunk, put them back
		if (ProcessedSamples < DataToProcess.Num())
		{
			const int32 RemainingSamples = DataToProcess.Num() - ProcessedSamples;
			TArray<float> RemainingData;
			RemainingData.SetNumUninitialized(RemainingSamples);
			FMemory::Memcpy(RemainingData.GetData(), DataToProcess.GetData() + ProcessedSamples, RemainingSamples * sizeof(float));
			
			// Put remaining data back at the beginning of the buffer
			FScopeLock Lock(&*DataGuard);
			AccumulatedAudioBuffer.Insert(RemainingData, 0);
		}
	}

	// Mark processing as complete
	bIsPipeProcessing.Store(false);

	// Check if we need to schedule another processing task
	bool bNeedsMoreProcessing = false;
	{
		FScopeLock Lock(&*DataGuard);
		bNeedsMoreProcessing = AccumulatedAudioBuffer.Num() >= ProcessingChunkSize;
	}

	// If there's more data to process and we're not already processing, schedule another task
	if (bNeedsMoreProcessing && !bIsPipeProcessing.Exchange(true))
	{
		AudioTaskPipe->Launch(AudioTaskPipe->GetDebugName(), [WeakThis = MakeWeakObjectPtr(this)]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->ProcessAccumulatedAudio();
			}
			else
			{
				UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to process accumulated audio because the generator object has been destroyed"));
			}
		}, UE::Tasks::ETaskPriority::BackgroundHigh);
	}
#endif
}

TMap<FString, float> URealisticMetaHumanLipSyncGenerator::GetControlValues() const
{
	FScopeLock Lock(&*DataGuard);
	return ControlValues;
}

void URealisticMetaHumanLipSyncGenerator::SetControlValues(TMap<FString, float> InValues)
{
	FScopeLock Lock(&*DataGuard);
	ControlValues = InValues;
}

TArray<FString> URealisticMetaHumanLipSyncGenerator::GetControlNames()
{
	static TArray<FString> ControlNames = {
		"CTRL_L_brow_down.ty", "CTRL_R_brow_down.ty", "CTRL_L_brow_lateral.ty", "CTRL_R_brow_lateral.ty",
		"CTRL_L_brow_raiseIn.ty", "CTRL_R_brow_raiseIn.ty", "CTRL_L_brow_raiseOut.ty", "CTRL_R_brow_raiseOut.ty",
		"CTRL_L_eye_blink.ty", "CTRL_R_eye_blink.ty", "CTRL_L_eye_squintInner.ty", "CTRL_R_eye_squintInner.ty",
		"CTRL_L_eye_cheekRaise.ty", "CTRL_R_eye_cheekRaise.ty", "CTRL_L_nose.ty", "CTRL_R_nose.ty",
		"CTRL_L_nose.tx", "CTRL_R_nose.tx", "CTRL_L_nose_nasolabialDeepen.ty", "CTRL_R_nose_nasolabialDeepen.ty",
		"CTRL_C_mouth.tx", "CTRL_L_mouth_upperLipRaise.ty", "CTRL_R_mouth_upperLipRaise.ty",
		"CTRL_L_mouth_lowerLipDepress.ty", "CTRL_R_mouth_lowerLipDepress.ty", "CTRL_L_mouth_cornerPull.ty",
		"CTRL_R_mouth_cornerPull.ty", "CTRL_L_mouth_stretch.ty", "CTRL_R_mouth_stretch.ty",
		"CTRL_L_mouth_dimple.ty", "CTRL_R_mouth_dimple.ty", "CTRL_L_mouth_cornerDepress.ty",
		"CTRL_R_mouth_cornerDepress.ty", "CTRL_L_mouth_purseU.ty", "CTRL_R_mouth_purseU.ty",
		"CTRL_L_mouth_purseD.ty", "CTRL_R_mouth_purseD.ty", "CTRL_L_mouth_towardsU.ty",
		"CTRL_R_mouth_towardsU.ty", "CTRL_L_mouth_towardsD.ty", "CTRL_R_mouth_towardsD.ty",
		"CTRL_L_mouth_funnelU.ty", "CTRL_R_mouth_funnelU.ty", "CTRL_L_mouth_funnelD.ty",
		"CTRL_R_mouth_funnelD.ty", "CTRL_L_mouth_lipsTogetherU.ty", "CTRL_R_mouth_lipsTogetherU.ty",
		"CTRL_L_mouth_lipsTogetherD.ty", "CTRL_R_mouth_lipsTogetherD.ty", "CTRL_L_mouth_lipBiteU.ty",
		"CTRL_R_mouth_lipBiteU.ty", "CTRL_L_mouth_lipBiteD.ty", "CTRL_R_mouth_lipBiteD.ty",
		"CTRL_L_mouth_sharpCornerPull.ty", "CTRL_R_mouth_sharpCornerPull.ty", "CTRL_L_mouth_pushPullU.ty",
		"CTRL_R_mouth_pushPullU.ty", "CTRL_L_mouth_pushPullD.ty", "CTRL_R_mouth_pushPullD.ty",
		"CTRL_L_mouth_cornerSharpnessU.ty", "CTRL_R_mouth_cornerSharpnessU.ty", "CTRL_L_mouth_cornerSharpnessD.ty",
		"CTRL_R_mouth_cornerSharpnessD.ty", "CTRL_L_mouth_lipsRollU.ty", "CTRL_R_mouth_lipsRollU.ty",
		"CTRL_L_mouth_lipsRollD.ty", "CTRL_R_mouth_lipsRollD.ty", "CTRL_C_jaw.ty", "CTRL_C_jaw.tx",
		"CTRL_C_jaw_fwdBack.ty", "CTRL_L_jaw_ChinRaiseD.ty", "CTRL_R_jaw_ChinRaiseD.ty", "CTRL_C_tongue_move.ty",
		"CTRL_C_tongue_move.tx", "CTRL_C_tongue_inOut.ty", "CTRL_C_tongue_tipMove.ty", "CTRL_C_tongue_tipMove.tx",
		"CTRL_C_tongue_wideNarrow.ty", "CTRL_C_tongue_press.ty", "CTRL_C_tongue_roll.ty", "CTRL_C_tongue_thickThin.ty"
	};

	return ControlNames;
}

bool URealisticMetaHumanLipSyncGenerator::InitializeModel()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	const FRuntimeMetaHumanLipSyncModule* RuntimeMetaHumanLipSyncModulePtr = static_cast<FRuntimeMetaHumanLipSyncModule*>(FModuleManager::Get().GetModule("RuntimeMetaHumanLipSync"));
	if (!RuntimeMetaHumanLipSyncModulePtr)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Cannot get the RuntimeMetaHumanLipSync module"));
		return false;
	}

	FSoftObjectPath VoiceModelAssetPath = FSoftObjectPath(RuntimeMetaHumanLipSyncModulePtr->GetModelDataAssetPath());
	UAssetManager& AssetManager = UAssetManager::Get();

	FAssetData VoiceModelAssetData;
	TSoftObjectPtr<URealisticMetaHumanLipSyncModel> LipSyncModel;
	if (!AssetManager.GetAssetDataForPath(VoiceModelAssetPath, VoiceModelAssetData))
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Failed to get the lip sync model asset data"));
		return false;
	}

	LipSyncModel = TSoftObjectPtr<URealisticMetaHumanLipSyncModel>(FSoftObjectPath(VoiceModelAssetPath));
	if (!LipSyncModel.LoadSynchronous())
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Failed to get the lip sync model"));
		return false;
	}

	// Get the specific model data based on the selected model type
	FByteBulkData* SelectedModelData = LipSyncModel->GetModelData(GeneratorConfig.ModelType);
	if (!SelectedModelData)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Failed to get model data for selected model type"));
		return false;
	}

	if (SelectedModelData->GetBulkDataSize() <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Selected model type has no data available"));
		return false;
	}

	if (ModelDataPtr)
	{
		FMemory::Free(ModelDataPtr);
		ModelDataPtr = nullptr;
	}

	SelectedModelData->GetCopy(&ModelDataPtr, true);
	const int64 ModelBulkDataSize = SelectedModelData->GetBulkDataSize();

	// Create optimized session options
	Ort::SessionOptions SessionOptions;

	// Optimization settings
	SessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
	SessionOptions.EnableMemPattern();
	SessionOptions.EnableCpuMemArena();

	// Configure thread settings
	int32 FinalIntraOpThreads = GeneratorConfig.IntraOpThreads;
	int32 FinalInterOpThreads = GeneratorConfig.InterOpThreads;

	if (FinalIntraOpThreads <= 0)
	{
		// Auto-detect: use 1/4 of available cores, max 4
		const int32 NumCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		FinalIntraOpThreads = FMath::Clamp(NumCores / 4, 1, 4);
	}

	if (FinalInterOpThreads <= 0)
	{
		// Auto-detect: use 1/8 of available cores, max 2
		const int32 NumCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		FinalInterOpThreads = FMath::Clamp(NumCores / 8, 1, 2);
	}

	SessionOptions.SetIntraOpNumThreads(FinalIntraOpThreads);
	SessionOptions.SetInterOpNumThreads(FinalInterOpThreads);

	// Create session from decoded model data
	OrtSession = MakeUnique<Ort::Session>(*OrtEnvironment, ModelDataPtr, ModelBulkDataSize, SessionOptions);

	UE_LOG(LogRuntimeMetaHumanLipSync, Log, TEXT("Successfully loaded Epic's lip sync model with %d intra-op threads and %d inter-op threads"), FinalIntraOpThreads, FinalInterOpThreads);

	return true;
#else
    UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Realistic MetaHuman Lip Sync is not supported on your current platform"));
    return false;
#endif
}

bool URealisticMetaHumanLipSyncGenerator::ProcessFrameWithOnnx(const TArray<float>& AudioData)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (!OrtSession)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("ONNX Session not initialized"));
		return false;
	}

	// Update audio buffer
	if (AudioBuffer.Num() == 16000)
	{
		const int32 NewDataSize = AudioData.Num();
		FMemory::Memmove(AudioBuffer.GetData(), AudioBuffer.GetData() + NewDataSize, (16000 - NewDataSize) * sizeof(float));
		FMemory::Memcpy(AudioBuffer.GetData() + (16000 - NewDataSize), AudioData.GetData(), NewDataSize * sizeof(float));
	}

	// Get allocator for this call
	Ort::AllocatorWithDefaultOptions Allocator;

	// Create input tensors - recreate each time to avoid thread safety issues
	std::vector<Ort::Value> InputTensors;
	std::vector<const char*> InputNames;
	std::vector<Ort::AllocatedStringPtr> InputNamePtrs;

	// Create memory info for this call
	Ort::MemoryInfo MemoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

	// Audio input tensor
	{
		Ort::AllocatedStringPtr InputNamePtr = OrtSession->GetInputNameAllocated(0, Allocator);
		InputNamePtrs.push_back(std::move(InputNamePtr));
		InputNames.push_back(InputNamePtrs.back().get());

		std::vector<int64_t> AudioShape = {16000};
		InputTensors.push_back(Ort::Value::CreateTensor<float>(MemoryInfo, AudioBuffer.GetData(), AudioBuffer.Num(), AudioShape.data(), AudioShape.size()));
	}

	// Curve values input tensor
	{
		Ort::AllocatedStringPtr InputNamePtr = OrtSession->GetInputNameAllocated(1, Allocator);
		InputNamePtrs.push_back(std::move(InputNamePtr));
		InputNames.push_back(InputNamePtrs.back().get());

		std::vector<int64_t> CurveShape = {81};
		InputTensors.push_back(Ort::Value::CreateTensor<float>(MemoryInfo, CurveValues.GetData(), CurveValues.Num(), CurveShape.data(), CurveShape.size()));
	}

	// Kalman buffer input tensor
	{
		Ort::AllocatedStringPtr InputNamePtr = OrtSession->GetInputNameAllocated(2, Allocator);
		InputNamePtrs.push_back(std::move(InputNamePtr));
		InputNames.push_back(InputNamePtrs.back().get());

		std::vector<int64_t> KalmanShape = {81, 81};
		InputTensors.push_back(Ort::Value::CreateTensor<float>(MemoryInfo, KalmanBuffer.GetData(), KalmanBuffer.Num(), KalmanShape.data(), KalmanShape.size()));
	}

	// Get output names
	std::vector<const char*> OutputNames;
	std::vector<Ort::AllocatedStringPtr> OutputNamePtrs;

	size_t NumOutputNodes = OrtSession->GetOutputCount();
	for (size_t i = 0; i < NumOutputNodes; i++)
	{
		Ort::AllocatedStringPtr OutputNamePtr = OrtSession->GetOutputNameAllocated(i, Allocator);
		OutputNamePtrs.push_back(std::move(OutputNamePtr));
		OutputNames.push_back(OutputNamePtrs.back().get());
	}

	auto OutputTensors = OrtSession->Run(Ort::RunOptions{nullptr}, InputNames.data(), InputTensors.data(),
									   InputTensors.size(), OutputNames.data(), OutputNames.size());

	// Process outputs
	if (OutputTensors.size() >= 2)
	{
		// Get curve values output
		float* CurveOutput = OutputTensors[0].GetTensorMutableData<float>();
		size_t CurveOutputCount = OutputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

		if (CurveOutputCount == 81)
		{
			// Update curve values
			FMemory::Memcpy(CurveValues.GetData(), CurveOutput, CurveOutputCount * sizeof(float));

			// Update control values map with reduced locking
			FScopeLock Lock(&*DataGuard);
			ControlValues.Empty();

			TArray<FString> ControlNames = GetControlNames();
			check(ControlNames.Num() == CurveValues.Num());

			for (int32 i = 0; i < ControlNames.Num(); ++i)
			{
				ControlValues.Add(ControlNames[i], CurveValues[i]);
			}
		}
		else
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unexpected curve output count: %zu (expected 81)"), CurveOutputCount);
			return false;
		}

		// Get Kalman buffer output
		float* KalmanOutput = OutputTensors[1].GetTensorMutableData<float>();
		size_t KalmanOutputCount = OutputTensors[1].GetTensorTypeAndShapeInfo().GetElementCount();

		if (KalmanOutputCount == 81 * 81)
		{
			FMemory::Memcpy(KalmanBuffer.GetData(), KalmanOutput, KalmanOutputCount * sizeof(float));
		}
		else
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unexpected Kalman output count: %zu (expected 6561)"), KalmanOutputCount);
			return false;
		}

		return true;
	}
	else
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unexpected number of outputs: %zu (expected at least 2)"), OutputTensors.size());
		return false;
	}
#else
	UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Realistic MetaHuman Lip Sync is not supported on your current platform"));
	return false;
#endif
}

bool URealisticMetaHumanLipSyncGenerator::ResampleAudioData(Audio::FAlignedFloatBuffer& RAWData, int32 NumOfChannels, int32 SourceSampleRate, int32 DestinationSampleRate, Audio::FAlignedFloatBuffer& ResampledRAWData)
{
	if (NumOfChannels <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to resample audio data because the number of channels is invalid (%d)"), NumOfChannels);
		return false;
	}
	if (SourceSampleRate <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to resample audio data because the source sample rate is invalid (%d)"), SourceSampleRate);
		return false;
	}
	if (DestinationSampleRate <= 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to resample audio data because the destination sample rate is invalid (%d)"), DestinationSampleRate);
		return false;
	}

	// No need to resample if the sample rates are the same
	if (SourceSampleRate == DestinationSampleRate)
	{
		ResampledRAWData = RAWData;
		return true;
	}

	const Audio::FResamplingParameters ResampleParameters = {
		Audio::EResamplingMethod::BestSinc,
		static_cast<int32>(NumOfChannels),
		static_cast<float>(SourceSampleRate),
		static_cast<float>(DestinationSampleRate),
		RAWData
	};

	ResampledRAWData.AddUninitialized(Audio::GetOutputBufferSize(ResampleParameters));
	Audio::FResamplerResults ResampleResults;
	ResampleResults.OutBuffer = &ResampledRAWData;

	if (!Audio::Resample(ResampleParameters, ResampleResults))
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to resample audio data from %d to %d"), SourceSampleRate, DestinationSampleRate);
		return false;
	}

	return true;
}

bool URealisticMetaHumanLipSyncGenerator::MixChannelsRAWData(Audio::FAlignedFloatBuffer& RAWData, int32 SampleRate, int32 SourceNumOfChannels, int32 DestinationNumOfChannels, Audio::FAlignedFloatBuffer& RemixedRAWData)
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
