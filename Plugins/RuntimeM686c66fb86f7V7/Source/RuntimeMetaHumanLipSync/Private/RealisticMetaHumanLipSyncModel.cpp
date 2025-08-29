// Georgy Treshchev 2025.

#include "RealisticMetaHumanLipSyncModel.h"

#include "RealisticMetaHumanLipSyncGenerator.h"
#include "RuntimeMetaHumanLipSyncModule.h"

FByteBulkData* URealisticMetaHumanLipSyncModel::GetModelData(ERealisticMetaHumanLipSyncModelType ModelType)
{
	switch (ModelType)
	{
	case ERealisticMetaHumanLipSyncModelType::Original:
		return &OriginalModel;
	case ERealisticMetaHumanLipSyncModelType::SemiOptimized:
		return &SemiOptimizedModel;
	case ERealisticMetaHumanLipSyncModelType::HighlyOptimized:
		return &HighlyOptimizedModel;
	}
	return nullptr;
}

void URealisticMetaHumanLipSyncModel::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Serialize all model variants
	OriginalModel.Serialize(Ar, this, INDEX_NONE, false);
	SemiOptimizedModel.Serialize(Ar, this, INDEX_NONE, false);
	HighlyOptimizedModel.Serialize(Ar, this, INDEX_NONE, false);

	UE_LOG(LogRuntimeMetaHumanLipSync, Log, TEXT("Serializing model data - Original: %lld bytes, Semi-Optimized: %lld bytes, Highly Optimized: %lld bytes"),
	       OriginalModel.GetBulkDataSize(), SemiOptimizedModel.GetBulkDataSize(), HighlyOptimizedModel.GetBulkDataSize());
}
