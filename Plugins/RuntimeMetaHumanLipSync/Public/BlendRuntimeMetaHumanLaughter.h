// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeVisemeGenerator.h"
#include "AnimNodes/AnimNode_PoseByName.h"
#include "BlendRuntimeMetaHumanLaughter.generated.h"

USTRUCT(BlueprintType)
struct FAnimNode_BlendRuntimeMetaHumanLaughter : public FAnimNode_PoseHandler
{
	GENERATED_BODY()

public:
	RUNTIMEMETAHUMANLIPSYNC_API FAnimNode_BlendRuntimeMetaHumanLaughter();
	
	// FAnimNode_Base interface
	RUNTIMEMETAHUMANLIPSYNC_API virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	RUNTIMEMETAHUMANLIPSYNC_API virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	RUNTIMEMETAHUMANLIPSYNC_API virtual void RebuildPoseList(const FBoneContainer& InBoneContainer, const UPoseAsset* InPoseAsset) override;

	RUNTIMEMETAHUMANLIPSYNC_API virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;

	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
	FPoseLink SourcePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, transient, Category=Copy, meta=(PinShownByDefault))
	TWeakObjectPtr<URuntimeVisemeGenerator> VisemeGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float InterpolationSpeed = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	float ResetTime = 0.2f;

	/** Scales the maximum intensity of the laughter animation (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxLaughterWeight = 0.7f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FName LaughterPoseName = FName("Laugh");
	
private:
	float TargetLaughterWeight = 0.0f;
	float CurrentLaughterWeight = 0.0f;
	float TimeSinceLastLaughterChange = 0.0f;
	FPoseCurve LaughterPoseCurve;
	bool bLaughterPoseFound = false;
};
