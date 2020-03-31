// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class GeoTempRoads : ModuleRules
{
	public GeoTempRoads(ReadOnlyTargetRules Target) : base(Target)
	{

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {
			// ... add public include paths required here ...
		});


		PrivateIncludePaths.AddRange(new string[] {
			// ... add other private include paths required here ...
		});


		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"RuntimeMeshComponent", 
			"GeoTempCore", 
			"GeoTempPostgis"
		});
		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"ProceduralMeshComponentEditor",
				"MeshDescription",
				"MeshDescriptionOperations",
				"AssetTools",
				"AssetRegistry",
			});
		}

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"GeoTempCore", 
			"GeoTempPostgis" 
		});		
	}
}
