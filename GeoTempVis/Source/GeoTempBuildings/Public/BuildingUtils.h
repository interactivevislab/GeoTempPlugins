#pragma once
#include "BuildingsData.h"
#include "ProceduralMeshComponent.h"
#include "RuntimeMeshComponent.h"
#include "BuildingUtils.generated.h"

USTRUCT()
struct FMeshSegmentData
{
	GENERATED_BODY()

	int32 SectionIndex = 0;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FVector2D> UV1;
	TArray<FVector2D> UV2;
	TArray<FVector2D> UV3;
	TArray<FLinearColor> VertexColors;
	UMaterialInterface* Material;
};

USTRUCT(BlueprintType)
struct FBuildingMeshData
{
	GENERATED_BODY()

		int LastFreeIndex = 0;
	TArray<FMeshSegmentData> Segments;

	void Append(FBuildingMeshData otherData) {
		LastFreeIndex = otherData.LastFreeIndex;
		Segments.Append(otherData.Segments);
	}

	void Clear() {
		LastFreeIndex = 0;
		Segments.Empty();
	}
};

UINTERFACE(MinimalAPI)
class URoofMaker : public UInterface
{
public:
	GENERATED_BODY()
};

class GEOTEMPBUILDINGS_API IRoofMaker
{
public:
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
	FBuildingMeshData GenerateRoof(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial);
};

class MeshHelpers {

	static TMap<FString, TScriptInterface<IRoofMaker>> RoofMakers;
	
public:

	inline static void AddRoofMaker(FString type, TScriptInterface<IRoofMaker> maker)
	{
		RoofMakers.Add(type, maker);
	}

	inline static bool CheckRoofMaker(FString type)
	{
		return RoofMakers.Contains(type);
	}

	inline static void SetRoofMakers(TMap<FString, TScriptInterface<IRoofMaker>> roofMakers)
	{
		RoofMakers = roofMakers;
	}
	
	static FBuildingMeshData CalculateMeshData(FBuildingPart* buildingPart, int firstSectionIndex, 
		UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial, FString flags = "");

	static void ConstructProceduralMesh(UProceduralMeshComponent* procMesh, FBuildingMeshData meshData);
	static void ConstructRuntimeMesh(URuntimeMeshComponent* meshComp, FBuildingMeshData meshData);
};