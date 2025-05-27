// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Escape : ModuleRules
{	public Escape(ReadOnlyTargetRules Target) : base(Target)
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
            
            // Ensure Objective-C is enabled for iOS
            bEnableObjCAutomaticReferenceCounting = true;
        }
        
        // Ensure clean separation between platforms
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            // Explicitly ensure no Objective-C for Android builds
            bEnableObjCAutomaticReferenceCounting = false;
        }

        // Only include Objective-C++ source files for iOS
        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PrivateIncludePaths.AddRange(new string[] { "Escape/Private/IOS" });
            PrivateDependencyModuleNames.AddRange(new string[] { });            PrivateIncludePathModuleNames.AddRange(new string[] { });
            PrivateDefinitions.Add("WITH_IOS_SPEECH=1");
            // .mm files in Private/IOS/ will be automatically included for iOS builds
        }
        else
        {
            PrivateDefinitions.Add("WITH_IOS_SPEECH=0");
        }
	}
}
