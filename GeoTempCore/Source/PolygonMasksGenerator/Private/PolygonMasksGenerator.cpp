// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PolygonMasksGenerator.h"


#include "GlobalShader.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FPolygonMasksGeneratorModule"

void FPolygonMasksGeneratorModule::StartupModule()
{
	
	FString shadersPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins/GeoTempCore/Shaders"));

	if (FPaths::DirectoryExists(shadersPath)) 
	{
		AddShaderSourceDirectoryMapping("/GameShaders", shadersPath);
	} else
	{
		shadersPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins/GeoTempPlugins/GeoTempCore/Shaders"));
		AddShaderSourceDirectoryMapping("/GameShaders", shadersPath);
	}
}

void FPolygonMasksGeneratorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPolygonMasksGeneratorModule, PolygonMasksGenerator)