// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "BlendRuntimeMetaHumanLaughter.h"
#include "AnimGraphNode_BlendRuntimeMetaHumanLaughter.generated.h"

/**
 * Blends the runtime meta human laughter node
 */
UCLASS(MinimalAPI)
class UAnimGraphNode_BlendRuntimeMetaHumanLaughter : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	/** The blend node */
	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_BlendRuntimeMetaHumanLaughter Node;

public:
	UAnimGraphNode_BlendRuntimeMetaHumanLaughter();
    
	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	// End of UAnimGraphNode_Base interface
};
