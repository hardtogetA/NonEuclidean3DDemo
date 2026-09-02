// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NonEuclidean3DDemo : ModuleRules
{
	public NonEuclidean3DDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NonEuclidean3DDemo",
			"NonEuclidean3DDemo/Variant_Horror",
			"NonEuclidean3DDemo/Variant_Horror/UI",
			"NonEuclidean3DDemo/Variant_Shooter",
			"NonEuclidean3DDemo/Variant_Shooter/AI",
			"NonEuclidean3DDemo/Variant_Shooter/UI",
			"NonEuclidean3DDemo/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
