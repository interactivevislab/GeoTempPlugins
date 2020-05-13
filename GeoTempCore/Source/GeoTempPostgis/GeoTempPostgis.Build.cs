// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GeoTempPostgis : ModuleRules
{
    private void CopyToBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string binariesDir = Path.Combine(ModulePath, "Binaries", Target.Platform.ToString());
        string filename = Path.GetFileName(Filepath);
        System.Console.Write("Copy from " + Filepath + " to " + Path.Combine(binariesDir, filename) + "\n");
        if (!Directory.Exists(binariesDir))
            Directory.CreateDirectory(binariesDir);
        var files = Directory.GetFiles(Path.GetDirectoryName(Filepath), ".dll");
        foreach (var file in files)
        {
            var filename1 = Path.GetFileName(file);
            if (!File.Exists(Path.Combine(binariesDir, filename1)))
                File.Copy(Path.Combine(Filepath, filename1), Path.Combine(binariesDir, filename1), true);
        }
    }

    private string ModulePath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..")); }
    }

    public GeoTempPostgis(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "GeoTempCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { "GeoTempCore" });

        var path = System.Environment.GetEnvironmentVariable("Path");
        System.Console.Write("====================\nLooking for postgres path\n");
        System.Console.Write("Path:\n");
        System.Console.Write(path+"\n");
        var sqlCandidates = System.Array.FindAll(path.Split(';'), a => a.Contains("PostgreSQL"));

        var sqlPath = "";
        foreach (var cand in sqlCandidates)
        {
            System.Console.Write("Path:" + cand + "\n");
            if (cand.EndsWith("lib"))
            {
                System.Console.Write("Accepted\n");
                sqlPath = cand;
                break;
            }
            else
            {
                System.Console.Write("Skipped\n");
            }
        }
        if (sqlPath.Equals(""))
        {
            System.Console.Write("No path found\n==========================\n");
            Definitions.Add("NOPOSTGRES=true");
        }
        else
        {

            PrivateIncludePaths.Add(Path.Combine(sqlPath, @"..\include"));
            PublicAdditionalLibraries.Add(Path.Combine(sqlPath, "libpq.lib"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/libpq.dll", Path.Combine(sqlPath, "libpq.dll"));
            CopyToBinaries(Path.Combine(sqlPath, "libpq.dll"), Target);
            System.Console.Write("Path found\n==========================\n");
            Definitions.Add("NOPOSTGRES=false");
        }

        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "../../", "sqlLibraries", "libpq.lib"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../../", "sqlInclude"));
        System.Console.Write("=========================================" + Path.Combine(ModuleDirectory, "../../", "sqlInclude") + "\n");



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
                 "CoreUObject",
                // ... add other public dependencies that you statically link with here ...
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                // ... add private dependencies that you statically link with here ...    
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}
