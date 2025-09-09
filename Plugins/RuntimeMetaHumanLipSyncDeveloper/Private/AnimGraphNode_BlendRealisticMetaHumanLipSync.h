// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "BlendRealisticMetaHumanLipSyncAnimNode.h"
#include "AnimGraphNode_BlendRealisticMetaHumanLipSync.generated.h"

UCLASS(MinimalAPI)
class UAnimGraphNode_BlendRealisticMetaHumanLipSync : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_BlendRealisticMetaHumanLipSync Node;

	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	// End of UEdGraphNode interface
};