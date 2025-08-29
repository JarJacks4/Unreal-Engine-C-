// Georgy Treshchev 2025.

using System.IO;
using UnrealBuildTool;

public class RuntimeMetaHumanLipSync : ModuleRules
{
    // In earlier UE versions (such as 4.27), the architecture string is empty.
	protected string ArchitectureString
	{
		get
		{
			if (Target.Architecture.ToString().Length > 0)
			{
				return Target.Architecture.ToString();
			}
			else
			{
				return "x64";
			}
		}
	}

	protected void LoadOnnxRuntime()
	{
		string PlatformDir = Target.Platform.ToString();
		string IncDirPath = Path.Combine(ModuleDirectory, "..", "ThirdParty", "Include", "onnxruntime");
		string LibDirPath = "";
		string RelativeLibDirPath = "";
		string SharedLibFileName = "UNSUPPORTED_PLATFORM";

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 6)
			{
				PublicDependencyModuleNames.Add("NNERuntimeORT");
				PublicDependencyModuleNames.Add("NNEOnnxruntime");
			}
			else
			{
				RelativeLibDirPath = Path.Combine("..", "ThirdParty", "Lib", PlatformDir, "onnxruntime", ArchitectureString);
				LibDirPath = Path.Combine(ModuleDirectory, RelativeLibDirPath);
				SharedLibFileName = "onnxruntime.dll";
				PublicAdditionalLibraries.Add(Path.Combine(LibDirPath, "onnxruntime.lib"));
				RuntimeDependencies.Add(Path.Combine(LibDirPath, SharedLibFileName));
				PublicDelayLoadDLLs.Add(SharedLibFileName);
				PublicIncludePaths.Add(IncDirPath);
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			RelativeLibDirPath = Path.Combine("..", "ThirdParty", "Lib", PlatformDir, "onnxruntime", ArchitectureString);
			LibDirPath = Path.Combine(ModuleDirectory, RelativeLibDirPath);
			SharedLibFileName = "libonnxruntime.so";
			PublicAdditionalLibraries.Add(Path.Combine(LibDirPath, SharedLibFileName));
			RuntimeDependencies.Add(Path.Combine(LibDirPath, "libonnxruntime.so.1"));
			RuntimeDependencies.Add(Path.Combine(LibDirPath, SharedLibFileName));
			PublicIncludePaths.Add(IncDirPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			RelativeLibDirPath = Path.Combine("..", "ThirdParty", "Lib", PlatformDir, "onnxruntime");
			LibDirPath = Path.Combine(ModuleDirectory, RelativeLibDirPath);
			SharedLibFileName = "libonnxruntime.dylib";
			PublicAdditionalLibraries.Add(Path.Combine(LibDirPath, SharedLibFileName));
			RuntimeDependencies.Add(Path.Combine(LibDirPath, "libonnxruntime.1.19.2.dylib"));
			RuntimeDependencies.Add(Path.Combine(LibDirPath, SharedLibFileName));
			PublicIncludePaths.Add(IncDirPath);
		}

		string RelativeSharedLibPath = Path.Combine(RelativeLibDirPath, SharedLibFileName).Replace('\\', '/');
		PublicDefinitions.Add("ORT_API_MANUAL_INIT");
		if (Target.Type != TargetType.Editor)
		{
			PublicDefinitions.Add("ORT_NO_EXCEPTIONS");
		}

		PublicDefinitions.Add("ONNXRUNTIME_SHAREDLIB_PATH_TTS=" + RelativeSharedLibPath);
	}
    
    public RuntimeMetaHumanLipSync(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"AnimGraphRuntime", "Projects" });
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string ThirdPartyDirectory = Path.Combine(ModuleDirectory, "..", "ThirdParty");
        PublicIncludePaths.Add(Path.Combine(ThirdPartyDirectory, "Include"));
        PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "AnimGraphRuntime", "SignalProcessing", "AudioPlatformConfiguration", "SignalProcessing" });

        LoadOnnxRuntime();
    }
}
