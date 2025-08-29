// Georgy Treshchev 2025.

#include "RuntimeMetaHumanLipSyncModule.h"
#include "Misc/EngineVersionComparison.h"

#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 6, 0)
#include "NNEOnnxruntime.h"
#elif PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include <onnxruntime_cxx_api.h>
#endif

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogRuntimeMetaHumanLipSync);

void FRuntimeMetaHumanLipSyncModule::StartupModule()
{
#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 6, 0)
	UE_LOG(LogRuntimeMetaHumanLipSync, Log, TEXT("Loading ONNX Runtime from NNEOnnxruntime module"));

	const FString PluginDir = IPluginManager::Get().FindPlugin("NNERuntimeORT")->GetBaseDir();
	const FString OrtSharedLibPath = FPaths::Combine(PluginDir, TEXT(PREPROCESSOR_TO_STRING(ONNXRUNTIME_SHAREDLIB_PATH)));

	FPlatformProcess::PushDllDirectory(*FPaths::GetPath(OrtSharedLibPath));
	OrtSharedLibHandle = FPlatformProcess::GetDllHandle(*OrtSharedLibPath);
	FPlatformProcess::PopDllDirectory(*FPaths::GetPath(OrtSharedLibPath));

	TUniquePtr<UE::NNEOnnxruntime::OrtApiFunctions> OrtApiFunctions = UE::NNEOnnxruntime::LoadApiFunctions(OrtSharedLibHandle);
	if (!OrtApiFunctions.IsValid())
	{
		UE_LOG(LogRuntimeMetaHumanLipSync, Fatal, TEXT("Failed to load ONNX Runtime shared library functions!"));
		return;
	}

	Ort::InitApi(OrtApiFunctions->OrtGetApiBase()->GetApi(ORT_API_VERSION));
#elif PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	const FString PluginDir = IPluginManager::Get().FindPlugin("RuntimeMetaHumanLipSync")->GetBaseDir();
	const FString OrtSharedLibFilePath = FPaths::Combine(PluginDir, TEXT("Source"), TEXT("RuntimeMetaHumanLipSync"), TEXT(PREPROCESSOR_TO_STRING(ONNXRUNTIME_SHAREDLIB_PATH_TTS)));
	const FString OrtSharedLibDirPath = FPaths::GetPath(OrtSharedLibFilePath);

	UE_LOG(LogRuntimeMetaHumanLipSync, Log, TEXT("Loading ONNX Runtime dynamic/shared library from '%s'"), *OrtSharedLibFilePath);

	FPlatformProcess::PushDllDirectory(*FPaths::GetPath(OrtSharedLibDirPath));
	OrtSharedLibHandle = FPlatformProcess::GetDllHandle(*OrtSharedLibFilePath);
	FPlatformProcess::PopDllDirectory(*FPaths::GetPath(OrtSharedLibDirPath));
	
	Ort::InitApi();
#endif
}

void FRuntimeMetaHumanLipSyncModule::ShutdownModule()
{
}


FString FRuntimeMetaHumanLipSyncModule::GetModelDataAssetName() const
{
	return TEXT("EpicMetaHumanLipSyncCpuModel");
}

FString FRuntimeMetaHumanLipSyncModule::GetModelDataPackagePath() const
{
	return TEXT("/RuntimeMetaHumanLipSync/RealisticModelData");
}

FString FRuntimeMetaHumanLipSyncModule::GetModelDataFullPackagePath() const
{
	return FPaths::Combine(GetModelDataPackagePath(), GetModelDataAssetName());
}

FString FRuntimeMetaHumanLipSyncModule::GetModelDataAssetPath() const
{
	const FString AssetName = GetModelDataAssetName();
	return FString::Printf(TEXT("%s.%s"), *FPaths::Combine(*GetModelDataPackagePath(), *AssetName), *AssetName);
}

IMPLEMENT_MODULE(FRuntimeMetaHumanLipSyncModule, RuntimeMetaHumanLipSync);
