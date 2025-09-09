// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "RealisticMetaHumanLipSyncGenerator.h"
#include "Animation/AnimNodeBase.h"
#include "BlendRealisticMetaHumanLipSyncAnimNode.generated.h"

USTRUCT(BlueprintType)
struct RUNTIMEMETAHUMANLIPSYNC_API FAnimNode_BlendRealisticMetaHumanLipSync : public FAnimNode_Base
{
    GENERATED_BODY()

public:
    FAnimNode_BlendRealisticMetaHumanLipSync();
    
    // FAnimNode_Base interface
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
    virtual void Evaluate_AnyThread(FPoseContext& Output) override;
    virtual void GatherDebugData(FNodeDebugData& DebugData) override;
    // End of FAnimNode_Base interface

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
    FPoseLink SourcePose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, transient, Category=Copy, meta=(PinShownByDefault))
    TWeakObjectPtr<URealisticMetaHumanLipSyncGenerator> LipSyncGenerator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float InterpolationSpeed = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ResetTime = 0.2f;

private:
    /** Current control values for smooth interpolation */
    TMap<FString, float> CurrentControlValues;
    
    /** Target control values from the lip sync generator */
    TMap<FString, float> TargetControlValues;
    
    /** Time since the last control values change */
    float TimeSinceLastChange = 0.0f;
};