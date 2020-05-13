// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GeoTempCore : ModuleRules
{
    public GeoTempCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                // ... add other public dependencies that you statically link with here ...
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                // ... add private dependencies that you statically link with here ...    
            }
            );
        
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );

        string libiglLibrariesPath = Path.Combine(ModuleDirectory, "../../", "libigl-static");

        PublicIncludePaths.Add(Path.Combine(libiglLibrariesPath, "include"));
        PublicIncludePaths.Add(Path.Combine(libiglLibrariesPath, @"external\triangle"));
        PublicIncludePaths.Add(Path.Combine(libiglLibrariesPath, @"external\eigen"));
        PublicIncludePaths.Add(Path.Combine(libiglLibrariesPath, @"external\glad\include\"));
        PublicIncludePaths.Add(Path.Combine(libiglLibrariesPath, @"external\glfw\include\"));


        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/triangle.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/igl_triangle.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/igl_opengl.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/igl_opengl_glfw.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/glad.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libiglLibrariesPath, "lib/glfw3.lib"));
    }
}
