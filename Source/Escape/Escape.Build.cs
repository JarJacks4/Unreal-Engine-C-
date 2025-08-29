// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Escape : ModuleRules
{
    public Escape(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Expose public headers (needed for EscapeUnrealWrapper.h)
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        // Public dependency modules
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "Json",
            "JsonUtilities",
            "HTTP",
            "NavigationSystem"
        });

        // Private dependency modules
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects",
            "Slate",
            "SlateCore",
            "Launch"   // needed for LaunchEngineLoop / engine boot
        });

        // iOS specifics
        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            bEnableObjCExceptions = true;

            // Add required frameworks for speech recognition
            PublicFrameworks.AddRange(new string[]
            {
                "Speech",
                "AVFoundation"
            });

            // Private include paths for iOS
            PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/IOS"));

            // Preprocessor definition
            PrivateDefinitions.Add("WITH_IOS_SPEECH=1");
        }
        else
        {
            PrivateDefinitions.Add("WITH_IOS_SPEECH=0");
        }
    }
}
