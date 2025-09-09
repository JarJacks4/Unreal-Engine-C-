// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tasks/Pipe.h"
#include "AudioResampler.h"
#include "SampleBuffer.h"
#include "Misc/EngineVersionComparison.h"
#include "Templates/Atomic.h"
#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 6, 0)
#include "NNEOnnxruntime.h"
#elif PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include <onnxruntime_cxx_api.h>
#endif
#include "RealisticMetaHumanLipSyncGenerator.generated.h"

/**
 * Model optimization levels for MetaHuman lip sync
 */
UENUM(BlueprintType, Category = "Runtime MetaHuman Lip Sync")
enum class ERealisticMetaHumanLipSyncModelType : uint8
{
    /** Original model - highest quality, highest CPU usage */
    Original UMETA(DisplayName = "Original (Highest Quality)"),
    
    /** Semi-optimized model - balanced quality and performance */
    SemiOptimized UMETA(DisplayName = "Semi-Optimized (Balanced)"),
    
    /** Highly optimized model - lowest CPU usage, may be less stable with noisy audio */
    HighlyOptimized UMETA(DisplayName = "Highly Optimized (Fastest)")
};

/**
 * Configuration structure for MetaHuman lip sync generator
 */
USTRUCT(BlueprintType)
struct FRealisticMetaHumanLipSyncConfig
{
    GENERATED_BODY()

    /**
     * Model type to use for lip sync generation
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model Configuration")
    ERealisticMetaHumanLipSyncModelType ModelType = ERealisticMetaHumanLipSyncModelType::HighlyOptimized;

    /**
     * Number of threads for intra-op parallelism. 
     * Set to 0 to use automatic detection (1/4 of available cores, max 4).
     * Higher values may improve performance on multi-core systems but use more CPU.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "16"))
    int32 IntraOpThreads = 0;

    /**
     * Number of threads for inter-op parallelism.
     * Set to 0 to use automatic detection (1/8 of available cores, max 2).
     * Usually kept low for real-time processing.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "8"))
    int32 InterOpThreads = 0;

    FRealisticMetaHumanLipSyncConfig()
    {
        ModelType = ERealisticMetaHumanLipSyncModelType::HighlyOptimized;
        IntraOpThreads = 0;
        InterOpThreads = 0;
    }
};

/**
 * Dedicated realistic (but slow) lip sync generator for MetaHuman characters
 */
UCLASS(BlueprintType)
class RUNTIMEMETAHUMANLIPSYNC_API URealisticMetaHumanLipSyncGenerator : public UObject
{
    GENERATED_BODY()
    
public:
    URealisticMetaHumanLipSyncGenerator();
    virtual ~URealisticMetaHumanLipSyncGenerator() override;

    /**
     * Create a new instance of the MetaHuman lip sync generator
     *
     * @param Config Configuration settings for the generator (optional)
     * @return Created MetaHuman lip sync generator
     */
    UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "Config"), Category = "MetaHuman Lip Sync Generator")
    static URealisticMetaHumanLipSyncGenerator* CreateRealisticMetaHumanLipSyncGenerator(UPARAM(DisplayName = "Configuration") const FRealisticMetaHumanLipSyncConfig& Config);

    /**
     * Processes raw PCM audio data to generate MetaHuman control values
     *
     * @param PCMData The raw PCM audio data to process
     * @param SampleRate The sample rate of the audio data
     * @param NumOfChannels The number of channels in the audio data
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman Lip Sync Generator")
    void ProcessAudioData(TArray<float> PCMData, int32 SampleRate, int32 NumOfChannels);

    /**
     * Retrieves the current MetaHuman control values
     *
     * @return A map of control names to values
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman Lip Sync Generator")
    TMap<FString, float> GetControlValues() const;

    void SetControlValues(TMap<FString, float> InValues);

    /**
     * Retrieves the names of the MetaHuman controls supported by the system
     *
     * @return An array of strings containing the names of the controls
     */
    UFUNCTION(BlueprintCallable, Category = "MetaHuman Lip Sync Generator")
    static TArray<FString> GetControlNames();

protected:
    /**
     * Configuration for this generator instance
     */
    FRealisticMetaHumanLipSyncConfig GeneratorConfig;

public:

    /**
     * Size of audio chunks processed in each inference step (in samples).
     * Default is 160 samples (10ms at 16kHz).
     * Smaller values provide more frequent updates but higher CPU usage.
     * Larger values reduce CPU load but may decrease lip sync responsiveness.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Lip Sync Generator", meta = (ClampMin = "80", ClampMax = "1600"))
    int32 ProcessingChunkSize = 160;

    /**
     * Maximum size of the accumulated audio buffer in samples.
     * This prevents unbounded memory growth if processing can't keep up.
     * When exceeded, oldest audio data is discarded.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Lip Sync Generator", meta = (ClampMin = "1600", ClampMax = "48000"))
    int32 MaxAccumulatedBufferSize = 16000; // 1 second at 16kHz

private:
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    /** ONNX Runtime environment */
    TSharedPtr<Ort::Env> OrtEnvironment;
    
    /** ONNX Runtime session */
    TUniquePtr<Ort::Session> OrtSession;
#endif
    
    /** Model buffers */
    TArray<float> AudioBuffer;     // 1.0s of audio (16000 mono samples at 16kHz)
    TArray<float> CurveValues;     // 81 curve values
    TArray<float> KalmanBuffer;    // 81x81 Kalman filter state
    
    /** The current MetaHuman control values calculated from the audio data */
    TMap<FString, float> ControlValues;

    /** Data guard (mutex) for thread safety */
    mutable TSharedPtr<FCriticalSection> DataGuard;

    /** Task pipe for handling audio processing tasks */
    TUniquePtr<UE::Tasks::FPipe> AudioTaskPipe;
    
    /** Initialize the ONNX model */
    bool InitializeModel();
    
    /** Process audio frame with ONNX model */
    bool ProcessFrameWithOnnx(const TArray<float>& AudioData);
    
    /** Resample and convert audio data */
    static bool ResampleAudioData(Audio::FAlignedFloatBuffer& RAWData, int32 NumOfChannels, int32 SourceSampleRate, int32 DestinationSampleRate, Audio::FAlignedFloatBuffer& ResampledRAWData);

    /** Mixing RAW Data to a different number of channels */
    static bool MixChannelsRAWData(Audio::FAlignedFloatBuffer& RAWData, int32 SampleRate, int32 SourceNumOfChannels, int32 DestinationNumOfChannels, Audio::FAlignedFloatBuffer& RemixedRAWData);

    void* ModelDataPtr = nullptr;

    /** Accumulated audio buffer for processing - protected by DataGuard */
    TArray<float> AccumulatedAudioBuffer;

    /** Flag to track if the pipe is currently processing */
    TAtomic<bool> bIsPipeProcessing{false};

    /** Process accumulated audio data in the pipe */
    void ProcessAccumulatedAudio();
};