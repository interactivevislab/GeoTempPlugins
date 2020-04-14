// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "FoliageInfo.generated.h"


USTRUCT(BlueprintType)
struct GEOTEMPFOLIAGE_API FFoliageMeshInfo
{
	GENERATED_BODY()
public:
	FFoliageMeshInfo();
	~FFoliageMeshInfo();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeshTransform")
	float MinScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform")
	float MaxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	float RotationOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	float MaxRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	bool UseRotationPresets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	TArray<float> RotationPresets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancingPattern")
	float Density;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancingPattern", meta = (UIMin = "0.0", UIMax = "1.0"))
	float Scatter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0.0", UIMax = "1.0"))
	float Z_AxisCorrection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinVertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseNormals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int, UMaterialInstanceDynamic*> MaterialInstances;
};
