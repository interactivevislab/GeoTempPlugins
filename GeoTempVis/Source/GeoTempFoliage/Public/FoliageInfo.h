// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "FoliageInfo.generated.h"


/**
 * \enum ELeafType
 *
 * What type of leaves mesh represents.
 */
UENUM(BlueprintType)
enum class ELeafType : uint8
{
	Broadleaved			UMETA(DisplayName = "Broadleaved"),		/** < Mesh represents broadleaved foliage. */
	Needleleaved		UMETA(DisplayName = "Needleleaved"),	/** < Mesh represents needleleaved foliage. */
};


/**
* \struct FFoliageMeshInfo
*
* Describes mesh instancing parameters.
*/
USTRUCT(BlueprintType)
struct GEOTEMPFOLIAGE_API FFoliageMeshInfo
{
	GENERATED_BODY()
public:
	FFoliageMeshInfo();
	~FFoliageMeshInfo();

public:
	/** Mesh to be instanced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* Mesh = nullptr;

	/** Type of leaves mesh represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	ELeafType LeafType;

	/** Minimum scale value to apply to mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeshTransform")
	float MinScale;

	/** Maximum scale value to apply to mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform")
	float MaxScale;

	/** Default mesh rotation angle (in degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	float RotationOrigin;

	/** 
	* Angle offset (in degrees) from RotationOrigin. 
	* Varialbe is used when UseRotationPresets is 'false'.
	*
	* @see FFoliageMeshInfo.RotationOrigin
	* @see FFoliageMeshInfo.UseRotationPresets
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	float MaxRotation;

	/** 
	* Determines whether to use RotationPresets or not. 
	*
	* @see FFoliageMeshInfo.RotationPresets
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	bool UseRotationPresets;

	/** 
	* A set of angle offsets (in degrees) to choose from instead of random angle offset via MaxRotation. 
	* Varialbe is used when UseRotationPresets is 'true'.
	*
	* @see FFoliageMeshInfo.MaxRotation
	* @see FFoliageMeshInfo.UseRotationPresets
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshTransform", meta = (UIMin = "0.0", UIMax = "360.0"))
	TArray<float> RotationPresets;

	/** Maximum number of meshes to spawn in a cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancingPattern")
	float Density;

	/** 
	* Cell placement scale. 
	* Value ranges between 0.0f and 1.0f where:
	* 1.0f means use all space in a cell and place mesh anywhere within it.
	* 0.0f means place mesh strictly in the center of the cell.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancingPattern", meta = (UIMin = "0.0", UIMax = "1.0"))
	float Scatter;

	/** Z-axis offset for mesh placement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0.0", UIMax = "1.0"))
	float Z_AxisCorrection;

	/** 
	* Lower edge of mesh placement in Z-axis. 
	*
	* OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHeight;

	/** 
	* Upper edge of mesh placement in Z-axis. 
	*
	* OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight;

	/** 
	* Minimum vertical value to place mesh. 
	*
	* OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinVertical;

	/** 
	* Whether to place mesh according to land normal. 
	*
	* OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseNormals;

	/** A set of dynamic material instances generated during mesh placement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int, UMaterialInstanceDynamic*> MaterialInstances;
};
