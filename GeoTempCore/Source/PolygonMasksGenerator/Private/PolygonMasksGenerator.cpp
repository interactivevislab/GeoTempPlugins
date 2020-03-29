// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PolygonMasksGenerator.h"


#include "GlobalShader.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FPolygonMasksGeneratorModule"

void FPolygonMasksGeneratorModule::StartupModule()
{
	FString shadersPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins/GeoTempCore/Shaders"));
	AddShaderSourceDirectoryMapping("/GameShaders", shadersPath);
}

void FPolygonMasksGeneratorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPolygonMasksGeneratorModule, PolygonMasksGenerator)