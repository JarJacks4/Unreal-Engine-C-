// Georgy Treshchev 2025.

#pragma once

#include "CoreMinimal.h"
#include "RealisticMetaHumanLipSyncModel.h"
#include "RuntimeMetaHumanLipSyncModule.h"
#include "Misc/EngineVersionComparison.h"
#include "Modules/ModuleManager.h"

RUNTIMEMETAHUMANLIPSYNC_API DECLARE_LOG_CATEGORY_EXTERN(LogRuntimeMetaHumanLipSync, Log, All);

class FRuntimeMetaHumanLipSyncModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual FString GetModelDataAssetName() const;
	virtual FString GetModelDataPackagePath() const;
	virtual FString GetModelDataFullPackagePath() const;
	virtual FString GetModelDataAssetPath() const;
protected:
	void* OrtSharedLibHandle = nullptr;
};