// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Escape : ModuleRules
{
	public Escape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "Json", "JsonUtilities", "HTTP", "NavigationSystem" });

        // iOS-specific frameworks for Speech Recognition - only for iOS builds
        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicFrameworks.AddRange(new string[]
            {
                "Speech",
                "AVFoundation"
            });
            
            // DO NOT enable ARC at module level - let individual .mm files handle it
            // bEnableObjCAutomaticReferenceCounting = true;
            
            // Include iOS-specific paths and definitions
            PrivateIncludePaths.AddRange(new string[] { "Escape/Private/IOS" });
            PrivateDefinitions.Add("WITH_IOS_SPEECH=1");
            
            // The .mm files will be automatically discovered in Private/IOS/ for iOS builds
        }
        else
        {
            PrivateDefinitions.Add("WITH_IOS_SPEECH=0");
        }
	}
}
