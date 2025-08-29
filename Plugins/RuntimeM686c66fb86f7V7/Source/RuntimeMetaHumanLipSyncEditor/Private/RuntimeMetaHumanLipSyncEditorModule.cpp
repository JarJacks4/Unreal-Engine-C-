// Georgy Treshchev 2025.

#include "Modules/ModuleManager.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Modules/ModuleManager.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformMisc.h"
#include "HAL/Platform.h"
#include "GameProjectGenerationModule.h"
#include "AssetRegistry/AssetRegistryModule.h"

#ifdef LOCTEXT_NAMESPACE
#undef LOCTEXT_NAMESPACE
#endif

#define LOCTEXT_NAMESPACE "FRuntimeMetaHumanLipSyncEditorModule"

RUNTIMEMETAHUMANLIPSYNCEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogRuntimeMetaHumanLipSyncEditor, Log, All);
DEFINE_LOG_CATEGORY(LogRuntimeMetaHumanLipSyncEditor);

static FString NNERuntimeORTString(TEXT("NNERuntimeORT"));

class FRuntimeMetaHumanLipSyncEditorModule : public IModuleInterface
{
public:
	/**
	 * Check if the conflicting plugin is enabled.
	 */
	virtual bool IsConflictingPluginEnabled()
	{
#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 6, 0)
		return false;
#else
		TSharedPtr<IPlugin> ConflictingPlugin = IPluginManager::Get().FindPlugin(NNERuntimeORTString);
		return ConflictingPlugin.IsValid() && ConflictingPlugin->IsEnabled();
#endif
	}

	/**
	 * Find all dependencies of the plugin.
	 */
	virtual void FindPluginDependencies(const FString& PluginName, TSet<FString>& Dependencies, const TMap<FString, IPlugin*>& NameToPlugin)
	{
		IPlugin* const* Plugin = NameToPlugin.Find(PluginName);
		if (Plugin != nullptr)
		{
			for (const FPluginReferenceDescriptor& Dependency : (*Plugin)->GetDescriptor().Plugins)
			{
				if (Dependency.bEnabled && !Dependencies.Contains(Dependency.Name))
				{
					Dependencies.Add(Dependency.Name);
					FindPluginDependencies(Dependency.Name, Dependencies, NameToPlugin);
				}
			}
		}
	}

	/**
	 * Disable conflicting plugin and restart the editor.
	 */
	virtual void DisableConflictingPluginAndRestart()
	{
		const FString ConflictingPluginName = NNERuntimeORTString;
    
		// Get all enabled plugins and build name to plugin map for dependency check
		TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();
		TMap<FString, IPlugin*> NameToPlugin;
		for (TSharedRef<IPlugin>& EnabledPlugin : EnabledPlugins)
		{
			NameToPlugin.FindOrAdd(EnabledPlugin->GetName()) = &(EnabledPlugin.Get());
		}

		// Find and collect all dependent plugins
		TArray<FString> PluginsToDisable;
		PluginsToDisable.Add(ConflictingPluginName);

		for (TSharedRef<IPlugin>& EnabledPlugin : EnabledPlugins)
		{
			FString EnabledPluginName = EnabledPlugin->GetName();
			TSet<FString> Dependencies;
			FindPluginDependencies(EnabledPluginName, Dependencies, NameToPlugin);

			if (Dependencies.Contains(ConflictingPluginName))
			{
				PluginsToDisable.AddUnique(EnabledPluginName);
			}
		}

		// Disable all collected plugins (main plugin and its dependents)
		bool bSuccess = true;
		FText FailMessage;
    
		for (const FString& PluginToDisable : PluginsToDisable)
		{
			if (!IProjectManager::Get().SetPluginEnabled(PluginToDisable, false, FailMessage))
			{
				bSuccess = false;
				break;
			}
		}

		// Save changes if successful
		if (bSuccess && IProjectManager::Get().IsCurrentProjectDirty())
		{
			FGameProjectGenerationModule::Get().TryMakeProjectFileWriteable(FPaths::GetProjectFilePath());
			bSuccess = IProjectManager::Get().SaveCurrentProjectToDisk(FailMessage);
		}

		if (!bSuccess)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailMessage);
			return;
		}

		// Request editor restart
		FUnrealEdMisc::Get().RestartEditor(false);
	}

	/**
	 * Show a modal dialog that informs the user about the conflicting plugin.
	 */
	virtual void ShowPluginConflictModal()
	{
		const FText Title = LOCTEXT("PluginConflictTitle", "Plugin Conflict Detected");
		const FText Message = LOCTEXT("PluginConflictMessage", 			"NNERuntimeORT plugin conflicts with RuntimeMetaHumanLipSync. It needs to be disabled for RuntimeMetaHumanLipSync to work correctly. Would you like to disable it and restart the editor?");

		const EAppReturnType::Type ModalResult = FMessageDialog::Open(
			EAppMsgType::YesNo,
			EAppReturnType::No,
			Message,
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			&Title
#else
			Title
#endif
			);

		if (ModalResult == EAppReturnType::Yes)
		{
			DisableConflictingPluginAndRestart();
		}
	}
	
	virtual void StartupModule() override
	{
		// Check if the conflicting plugin is enabled
		if (IsConflictingPluginEnabled())
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
			IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
			AssetRegistry.OnFilesLoaded().AddLambda([this]()
			{
				ShowPluginConflictModal();
			});
		}

		UpdatePackagingSettings();
	}

	bool UpdatePackagingSettings() const
	{
		UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
		if (!PackagingSettings)
		{
			UE_LOG(LogRuntimeMetaHumanLipSyncEditor, Error, TEXT("Cannot get the packaging settings"));
			return false;
		}

		auto AddPathIfNotPresent = [&PackagingSettings](const FString& NewPath)
		{
			const bool bIsAlreadyInPath = PackagingSettings->DirectoriesToAlwaysCook.ContainsByPredicate([&NewPath](const FDirectoryPath& DirPath)
			{
				return FPaths::IsSamePath(DirPath.Path, NewPath);
			});

			if (!bIsAlreadyInPath)
			{
				FDirectoryPath DirPath;
				DirPath.Path = NewPath;
				PackagingSettings->DirectoriesToAlwaysCook.Add(DirPath);
			}
		};

		AddPathIfNotPresent(TEXT("/RuntimeMetaHumanLipSync/RealisticModelData"));
		AddPathIfNotPresent(TEXT("/RuntimeMetaHumanLipSync/ModelData"));

		PackagingSettings->bCookMapsOnly = false;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
		PackagingSettings->UpdateDefaultConfigFile();
#else
		PackagingSettings->TryUpdateDefaultConfigFile();
#endif

		return true;
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRuntimeMetaHumanLipSyncEditorModule, RuntimeMetaHumanLipSyncEditor);
