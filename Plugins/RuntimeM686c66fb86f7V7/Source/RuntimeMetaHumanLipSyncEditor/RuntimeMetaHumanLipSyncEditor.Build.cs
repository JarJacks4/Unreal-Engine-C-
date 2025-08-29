// Georgy Treshchev 2025.

using System.IO;
using UnrealBuildTool;

public class RuntimeMetaHumanLipSyncEditor : ModuleRules
{
	public RuntimeMetaHumanLipSyncEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
		});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"Json",
				"InputCore",
				"Projects",
				"GameProjectGeneration",
				"DesktopPlatform"
			}
		);

		if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 0)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"DeveloperToolSettings"
				}
			);
		}
	}
}