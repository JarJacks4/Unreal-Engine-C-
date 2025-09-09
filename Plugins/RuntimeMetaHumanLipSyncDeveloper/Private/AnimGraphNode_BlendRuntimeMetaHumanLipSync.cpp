// Georgy Treshchev 2025.


#include "AnimGraphNode_BlendRuntimeMetaHumanLipSync.h"

#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "RuntimeLipSyncMetaHuman"

UAnimGraphNode_BlendRuntimeMetaHumanLipSync::UAnimGraphNode_BlendRuntimeMetaHumanLipSync()
{
	static ConstructorHelpers::FObjectFinder<UPoseAsset> DefaultPoseAsset(TEXT("/RuntimeMetaHumanLipSync/LipSyncData/LipsAnimationPoseAsset.LipsAnimationPoseAsset"));
	if (DefaultPoseAsset.Succeeded())
	{
		Node.PoseAsset = DefaultPoseAsset.Object;
	}
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLipSync::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLipSync::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_BlendRuntimeMetaHumanLipSync_Title", "Blend Runtime MetaHuman Lip Sync");
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLipSync::GetMenuCategory() const
{
	return LOCTEXT("AnimGraphNode_BlendRuntimeMetaHumanLipSync_Category", "Animation|RuntimeMetaHumanLipSync");
}

#undef LOCTEXT_NAMESPACE
