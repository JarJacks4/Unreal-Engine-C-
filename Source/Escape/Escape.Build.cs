// Copyright Epic Games, Inc.

using UnrealBuildTool;
using System.IO;

public class Escape : ModuleRules
{
    public Escape(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Json",
            "JsonUtilities",
            "HTTP",
            "NavigationSystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects",
            "Slate",
            "SlateCore"
        });

        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            bEnableObjCExceptions = true;

            PublicFrameworks.AddRange(new string[]
            {
                "Speech",
                "AVFoundation"
            });

            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/IOS"));
            PrivateDefinitions.Add("WITH_IOS_SPEECH=1");
        }
        else
        {
            PrivateDefinitions.Add("WITH_IOS_SPEECH=0");
        }
    }
}
