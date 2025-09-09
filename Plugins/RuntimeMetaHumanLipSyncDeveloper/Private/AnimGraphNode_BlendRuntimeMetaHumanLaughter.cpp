// Georgy Treshchev 2025.


#include "AnimGraphNode_BlendRuntimeMetaHumanLaughter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "RuntimeLipSyncMetaHuman"

UAnimGraphNode_BlendRuntimeMetaHumanLaughter::UAnimGraphNode_BlendRuntimeMetaHumanLaughter()
{
    static ConstructorHelpers::FObjectFinder<UPoseAsset> DefaultPoseAsset(TEXT("/RuntimeMetaHumanLipSync/LipSyncData/LaughterAnimationPoseAsset.LaughterAnimationPoseAsset"));
    if (DefaultPoseAsset.Succeeded())
    {
        Node.PoseAsset = DefaultPoseAsset.Object;
    }
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLaughter::GetTooltipText() const
{
    return LOCTEXT("AnimGraphNode_BlendRuntimeMetaHumanLaughter_Tooltip", "Blends MetaHuman laughter animation based on the laughter score from the Viseme Generator");
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLaughter::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("AnimGraphNode_BlendRuntimeMetaHumanLaughter_Title", "Blend Runtime MetaHuman Laughter");
}

FText UAnimGraphNode_BlendRuntimeMetaHumanLaughter::GetMenuCategory() const
{
    return LOCTEXT("AnimGraphNode_BlendRuntimeMetaHumanLaughter_Category", "Animation|RuntimeMetaHumanLipSync");
}

FString UAnimGraphNode_BlendRuntimeMetaHumanLaughter::GetNodeCategory() const
{
    return TEXT("Animation|Blend Poses");
}

void UAnimGraphNode_BlendRuntimeMetaHumanLaughter::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    Super::CustomizeDetails(DetailBuilder);

    TSharedRef<IPropertyHandle> LaughterPoseNameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FAnimNode_BlendRuntimeMetaHumanLaughter, LaughterPoseName));
    TSharedRef<IPropertyHandle> PoseAssetProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FAnimNode_PoseHandler, PoseAsset));
    
    IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Laughter Settings");
    CategoryBuilder.AddProperty(PoseAssetProperty);
    CategoryBuilder.AddProperty(LaughterPoseNameProperty)
        .ToolTip(LOCTEXT("LaughterPoseName_Tooltip", "Name of the pose in the PoseAsset that represents laughter (default is 'Laugh')"));
}

#undef LOCTEXT_NAMESPACE