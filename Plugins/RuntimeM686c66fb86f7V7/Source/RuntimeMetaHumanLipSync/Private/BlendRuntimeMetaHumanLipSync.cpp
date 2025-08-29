// Georgy Treshchev 2025.

#include "BlendRuntimeMetaHumanLipSync.h"

#include "RuntimeMetaHumanLipSyncModule.h"
#include "Animation/AnimInstanceProxy.h"
#include "Misc/EngineVersionComparison.h"

FAnimNode_BlendRuntimeMetaHumanLipSync::FAnimNode_BlendRuntimeMetaHumanLipSync()
{
	// Initialize with 15 zeros
	TargetVisemeWeights.Init(0.0f, 15);
	CurrentVisemeWeights.Init(0.0f, 15);
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	PoseByNameNodes.Empty();
	TArray<FString> VisemeNames = URuntimeVisemeGenerator::GetVisemeNames();
	for (const FString& VisemeName : VisemeNames)
	{
		FAnimNode_PoseByName PoseByNameNode;
		PoseByNameNode.PoseName = FName(*VisemeName);
		PoseByNameNodes.Add(PoseByNameNode);
	}

	SourcePose.Initialize(Context);

	RebuildPoseList(Context.AnimInstanceProxy->GetRequiredBones(), PoseAsset);
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	SourcePose.CacheBones(Context);
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::Evaluate_AnyThread(FPoseContext& Output)
{
	SourcePose.Evaluate(Output);

	if (!VisemeGenerator.IsValid())
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, VeryVerbose, TEXT("Unable to blend lip sync for MetaHuman because the VisemeGenerator is invalid"));
		return;
	}

	if (CurrentVisemeWeights.Num() != PoseByNameNodes.Num())
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend lip sync for MetaHuman because the number of visemes is invalid: (CurrentVisemeWeights.Num: %d, PoseByNameNodes.Num: %d)"), CurrentVisemeWeights.Num(), PoseByNameNodes.Num());
		return;
	}

	const UPoseAsset* CachedPoseAsset = CurrentPoseAsset.Get();
	if (CachedPoseAsset && PoseExtractContext.PoseCurves.Num() == PoseByNameNodes.Num())
	{
		// Apply interpolated weights to pose curves
		for (int32 Index = 0; Index < PoseByNameNodes.Num(); ++Index)
		{
			PoseExtractContext.PoseCurves[Index].Value = CurrentVisemeWeights[Index];
		}

		FAnimationPoseData OutputAnimationPoseData(Output);
		CurrentPoseAsset->GetAnimationPose(OutputAnimationPoseData, PoseExtractContext);
	}
	else
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend lip sync for MetaHuman because either the pose asset or the number of visemes is invalid: (CurrentPoseAsset.IsValid: %d, PoseByNameNodes.Num: %d)"), CurrentPoseAsset.IsValid(), PoseByNameNodes.Num());
		Output.ResetToRefPose();
	}
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::GatherDebugData(FNodeDebugData& DebugData)
{
	FAnimNode_Base::GatherDebugData(DebugData);
	SourcePose.GatherDebugData(DebugData.BranchFlow(1.f));
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::RebuildPoseList(const FBoneContainer& InBoneContainer, const UPoseAsset* InPoseAsset)
{
	if (!InPoseAsset)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend lip sync for MetaHuman because the pose asset is invalid"));
		return;
	}
	
#if UE_VERSION_NEWER_THAN(5, 2, 9)
	const TArray<FName>& PoseNames = InPoseAsset->GetPoseFNames();
#else
	const TArray<FSmartName> PoseNames = InPoseAsset->GetPoseNames();
#endif

	if (PoseNames.Num() == 0)
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend lip sync for MetaHuman because the pose asset has no poses"));
		return;
	}

	PoseExtractContext.PoseCurves.Reset();

	for (const FAnimNode_PoseByName& PoseByNameNode : PoseByNameNodes)
	{
		const int32 PoseIndex = InPoseAsset->GetPoseIndexByName(PoseByNameNode.PoseName);
		if (PoseIndex != INDEX_NONE)
		{
			if (PoseNames.IsValidIndex(PoseIndex))
			{
				PoseExtractContext.PoseCurves.Add(FPoseCurve(PoseIndex,
#if UE_VERSION_NEWER_THAN(5, 2, 9)
					PoseNames[PoseIndex],
#else
					PoseNames[PoseIndex].UID,
#endif
					0.f));
			}
		}
		else
		{
			UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend lip sync for MetaHuman because the pose name is invalid: %s"), *PoseByNameNode.PoseName.ToString());
		}
	}
}

void FAnimNode_BlendRuntimeMetaHumanLipSync::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	FAnimNode_PoseHandler::UpdateAssetPlayer(Context);
	SourcePose.Update(Context);

	// Rebuild pose list if needed
	if (PoseExtractContext.PoseCurves.Num() == 0)
	{
		RebuildPoseList(Context.AnimInstanceProxy->GetRequiredBones(), PoseAsset);
	}

	// Update target weights from VisemeGenerator
	if (VisemeGenerator.IsValid() && VisemeGenerator->GetVisemeWeights().Num() == 15)
	{
		TArray<float> VisemeWeights = VisemeGenerator->GetVisemeWeights();
		if (VisemeWeights.Num() == PoseByNameNodes.Num())
		{
			// Check if visemes have changed
			bool bVisemesChanged = false;
			for (int32 Index = 0; Index < PoseByNameNodes.Num(); ++Index)
			{
				if (FMath::Abs(VisemeWeights[Index] - TargetVisemeWeights[Index]) > KINDA_SMALL_NUMBER)
				{
					bVisemesChanged = true;
					break;
				}
			}

			// Update target weights and reset timer if visemes changed
			if (bVisemesChanged)
			{
				TargetVisemeWeights = MoveTemp(VisemeWeights);
				TimeSinceLastVisemeChange = 0.0f;
			}
			else
			{
				// Increment time since last change
				TimeSinceLastVisemeChange += Context.GetDeltaTime();

				// Reset to zero if no change for specified duration
				if (TimeSinceLastVisemeChange >= ResetTime)
				{
					TargetVisemeWeights.Init(0.0f, PoseByNameNodes.Num());
					VisemeGenerator->SetVisemeWeights(TargetVisemeWeights);
				}
			}
		}
	}

	// Smoothly interpolate current weights towards target weights
	const float DeltaTime = Context.GetDeltaTime();
	for (int32 Index = 0; Index < PoseByNameNodes.Num(); ++Index)
	{
		CurrentVisemeWeights[Index] = FMath::FInterpTo(
			CurrentVisemeWeights[Index],
			TargetVisemeWeights[Index],
			DeltaTime,
			InterpolationSpeed
		);
	}
}
