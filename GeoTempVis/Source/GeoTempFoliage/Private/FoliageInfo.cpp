// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageInfo.h"

FFoliageMeshInfo::FFoliageMeshInfo()
{
	Mesh				= nullptr;

	MinScale			= 1.0f;
	MaxScale			= 1.0f;
	RotationOrigin		= 0.0f;
	MaxRotation			= 0.0f;
	UseRotationPresets	= false;
	RotationPresets		= TArray<float>();

	Density				= 0.0f;
	Scatter				= 1.0f;

	Z_AxisCorrection	= 0.0f;
	MinHeight			= -1.0f;
	MaxHeight			= 10000.0f;
	MinVertical			= 0.9f;
	UseNormals			= false;

	MaterialInstances	= TMap<int, UMaterialInstanceDynamic*>();
}


FFoliageMeshInfo::~FFoliageMeshInfo()
{
}
