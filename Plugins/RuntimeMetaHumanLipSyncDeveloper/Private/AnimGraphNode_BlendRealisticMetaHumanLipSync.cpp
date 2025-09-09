// Georgy Treshchev 2025.

#include "AnimGraphNode_BlendRealisticMetaHumanLipSync.h"

#define LOCTEXT_NAMESPACE "MetaHumanLipSync"

FText UAnimGraphNode_BlendRealisticMetaHumanLipSync::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_BlendRealisticMetaHumanLipSync_Tooltip", "Controls MetaHuman face animation using audio input");
}

FText UAnimGraphNode_BlendRealisticMetaHumanLipSync::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_BlendRealisticMetaHumanLipSync_Title", "Blend Realistic MetaHuman Lip Sync");
}

FText UAnimGraphNode_BlendRealisticMetaHumanLipSync::GetMenuCategory() const
{
	return LOCTEXT("AnimGraphNode_BlendRealisticMetaHumanLipSync_Category", "Animation|MetaHuman");
}

#undef LOCTEXT_NAMESPACE