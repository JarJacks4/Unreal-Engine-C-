// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "BlendRuntimeMetaHumanLipSync.h"
#include "AnimGraphNode_BlendRuntimeMetaHumanLipSync.generated.h"

/**
 * Blends the runtime meta human lip sync node with the blend space
 */
UCLASS(MinimalAPI)
class UAnimGraphNode_BlendRuntimeMetaHumanLipSync : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	/** The blend node */
	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_BlendRuntimeMetaHumanLipSync Node;

public:
	UAnimGraphNode_BlendRuntimeMetaHumanLipSync();
	
	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	// End of UEdGraphNode interface
};
