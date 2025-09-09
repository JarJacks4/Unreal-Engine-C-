// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeVisemeGenerator.h"
#include "AnimNodes/AnimNode_PoseByName.h"
#include "BlendRuntimeMetaHumanLipSync.generated.h"

USTRUCT(BlueprintType)
struct FAnimNode_BlendRuntimeMetaHumanLipSync : public FAnimNode_PoseHandler
{
	GENERATED_BODY()

public:
	RUNTIMEMETAHUMANLIPSYNC_API FAnimNode_BlendRuntimeMetaHumanLipSync();
	
	// FAnimNode_Base interface
	RUNTIMEMETAHUMANLIPSYNC_API virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	RUNTIMEMETAHUMANLIPSYNC_API virtual void RebuildPoseList(const FBoneContainer& InBoneContainer, const UPoseAsset* InPoseAsset) override;

	RUNTIMEMETAHUMANLIPSYNC_API virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;

	TArray<FAnimNode_PoseByName> PoseByNameNodes;

	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
	FPoseLink SourcePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, transient, Category=Copy, meta=(PinShownByDefault))
	TWeakObjectPtr<URuntimeVisemeGenerator> VisemeGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float InterpolationSpeed = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float ResetTime = 0.2f;
	
private:
	TArray<float> TargetVisemeWeights;
	TArray<float> CurrentVisemeWeights;
	float TimeSinceLastVisemeChange = 0.0f;
};
