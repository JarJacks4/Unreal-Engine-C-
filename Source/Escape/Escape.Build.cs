// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Escape : ModuleRules
{
	public Escape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "Json", "JsonUtilities", "HTTP", "NavigationSystem" });
	}
}
