// Georgy Treshchev 2025.

#include "BlendRuntimeMetaHumanLaughter.h"

#include "RuntimeMetaHumanLipSyncModule.h"
#include "Animation/AnimInstanceProxy.h"
#include "Misc/EngineVersionComparison.h"

FAnimNode_BlendRuntimeMetaHumanLaughter::FAnimNode_BlendRuntimeMetaHumanLaughter()
{
    // Initialize with zeroed weights
    TargetLaughterWeight = 0.0f;
    CurrentLaughterWeight = 0.0f;
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
    SourcePose.Initialize(Context);
    RebuildPoseList(Context.AnimInstanceProxy->GetRequiredBones(), PoseAsset);
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
    SourcePose.CacheBones(Context);
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::Evaluate_AnyThread(FPoseContext& Output)
{
    SourcePose.Evaluate(Output);

    if (!VisemeGenerator.IsValid())
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, VeryVerbose, TEXT("Unable to blend laughter for MetaHuman because the VisemeGenerator is invalid"));
        return;
    }

    const UPoseAsset* CachedPoseAsset = CurrentPoseAsset.Get();
    if (CachedPoseAsset && bLaughterPoseFound)
    {
        // Apply interpolated weight to pose curve
        PoseExtractContext.PoseCurves.Empty(1);
        LaughterPoseCurve.Value = CurrentLaughterWeight;
        PoseExtractContext.PoseCurves.Add(LaughterPoseCurve);

        FAnimationPoseData OutputAnimationPoseData(Output);
        CurrentPoseAsset->GetAnimationPose(OutputAnimationPoseData, PoseExtractContext);
    }
    else if (!bLaughterPoseFound)
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend laughter for MetaHuman because the laughter pose '%s' was not found in the pose asset"), *LaughterPoseName.ToString());
    }
    else
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend laughter for MetaHuman because the pose asset is invalid"));
        Output.ResetToRefPose();
    }
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::GatherDebugData(FNodeDebugData& DebugData)
{
    FAnimNode_Base::GatherDebugData(DebugData);
    FString DebugLine = FString::Printf(TEXT("Laughter Weight: %.2f"), CurrentLaughterWeight);
    DebugData.AddDebugItem(DebugLine);
    SourcePose.GatherDebugData(DebugData.BranchFlow(1.f));
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::RebuildPoseList(const FBoneContainer& InBoneContainer, const UPoseAsset* InPoseAsset)
{
    if (!InPoseAsset)
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend laughter for MetaHuman because the pose asset is invalid"));
        bLaughterPoseFound = false;
        return;
    }
    
#if UE_VERSION_NEWER_THAN(5, 2, 9)
    const TArray<FName>& PoseNames = InPoseAsset->GetPoseFNames();
#else
    const TArray<FSmartName> PoseNames = InPoseAsset->GetPoseNames();
#endif

    if (PoseNames.Num() == 0)
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend laughter for MetaHuman because the pose asset has no poses"));
        bLaughterPoseFound = false;
        return;
    }

    PoseExtractContext.PoseCurves.Reset();

    const int32 PoseIndex = InPoseAsset->GetPoseIndexByName(LaughterPoseName);
    if (PoseIndex != INDEX_NONE)
    {
        if (PoseNames.IsValidIndex(PoseIndex))
        {
            LaughterPoseCurve = FPoseCurve(PoseIndex,
#if UE_VERSION_NEWER_THAN(5, 2, 9)
                PoseNames[PoseIndex],
#else
                PoseNames[PoseIndex].UID,
#endif
                0.f);
            bLaughterPoseFound = true;
        }
    }
    else
    {
        UE_LOG(LogRuntimeMetaHumanLipSync, Error, TEXT("Unable to blend laughter for MetaHuman because the laughter pose name is invalid: %s"), *LaughterPoseName.ToString());
        bLaughterPoseFound = false;
    }
}

void FAnimNode_BlendRuntimeMetaHumanLaughter::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
    FAnimNode_PoseHandler::UpdateAssetPlayer(Context);
    SourcePose.Update(Context);

    // Rebuild pose list if needed
    if (PoseExtractContext.PoseCurves.Num() == 0 && !bLaughterPoseFound)
    {
        RebuildPoseList(Context.AnimInstanceProxy->GetRequiredBones(), PoseAsset);
    }

    // Update target weights from VisemeGenerator
    if (VisemeGenerator.IsValid())
    {
        // Get laughter score from the VisemeGenerator
        const float LaughterScore = VisemeGenerator->GetLaughterScore();
        
        // Check if laughter has changed
        if (FMath::Abs(LaughterScore - TargetLaughterWeight) > KINDA_SMALL_NUMBER)
        {
            TargetLaughterWeight = FMath::Clamp(LaughterScore * MaxLaughterWeight, 0.0f, 1.0f);
            TimeSinceLastLaughterChange = 0.0f;
        }
        else
        {
            // Increment time since last change
            TimeSinceLastLaughterChange += Context.GetDeltaTime();

            // Reset to zero if no change for specified duration
            if (TimeSinceLastLaughterChange >= ResetTime && TargetLaughterWeight > 0.0f)
            {
                TargetLaughterWeight = 0.0f;
            }
        }
    }

    // Smoothly interpolate current weight towards target weight
    const float DeltaTime = Context.GetDeltaTime();
    CurrentLaughterWeight = FMath::FInterpTo(
        CurrentLaughterWeight,
        TargetLaughterWeight,
        DeltaTime,
        InterpolationSpeed
    );
}