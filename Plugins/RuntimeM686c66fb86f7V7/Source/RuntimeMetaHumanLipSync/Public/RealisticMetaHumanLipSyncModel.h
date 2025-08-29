// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Serialization/BulkData.h"
#include "RealisticMetaHumanLipSyncModel.generated.h"

enum class ERealisticMetaHumanLipSyncModelType : uint8;

/**
 * Realistic MetaHuman-only lip sync model
 */
UCLASS()
class RUNTIMEMETAHUMANLIPSYNC_API URealisticMetaHumanLipSyncModel : public UObject
{
	GENERATED_BODY()

public:
	/** Original model (best quality, highest CPU usage) */
	FByteBulkData OriginalModel;

	/** Semi-optimized model (good quality, medium CPU usage) */
	FByteBulkData SemiOptimizedModel;

	/** Highly optimized model (lower quality, lowest CPU usage) */
	FByteBulkData HighlyOptimizedModel;

	FByteBulkData* GetModelData(ERealisticMetaHumanLipSyncModelType ModelType);

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface
};
