// Georgy Treshchev 2025.

using System.IO;
using UnrealBuildTool;

public class RuntimeMetaHumanLipSyncDeveloper : ModuleRules
{
    public RuntimeMetaHumanLipSyncDeveloper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new string[] {
          "Core",
          "CoreUObject",
          "RuntimeMetaHumanLipSync",
          "AnimGraphRuntime",
          "AnimGraph",
          "BlueprintGraph", "Engine"
        });
    }
}

