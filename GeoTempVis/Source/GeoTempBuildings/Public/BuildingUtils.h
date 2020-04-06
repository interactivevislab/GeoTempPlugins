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

	inline void Append(FBuildingMeshData otherData) {
		LastFreeIndex = otherData.LastFreeIndex;
		Segments.Append(otherData.Segments);
	}

	inline void Clear() {
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
	FBuildingMeshData GenerateRoof(FBuildingPart inBuildingPart, int inFirstSectionIndex, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial);
};

class MeshHelpers {

	static TMap<FString, TScriptInterface<IRoofMaker>> RoofMakers;
	
public:

	inline static TTuple<FVector, FVector> GetNeighbourDirections(TArray<FVector> inContour, int inIndex)
	{
		int iprev = (inIndex + inContour.Num() - 1)	% inContour.Num();
		int inext = (inIndex + 1)					% inContour.Num();
		
		auto toPrev = inContour[inIndex] - inContour[iprev];
		auto toNext = inContour[inext] - inContour[inIndex];

		return MakeTuple(toPrev, toNext);
	}
	
	inline static void AddRoofMaker(FString inType, TScriptInterface<IRoofMaker> inMaker)
	{
		RoofMakers.Add(inType, inMaker);
	}

	inline static bool CheckRoofMaker(FString inType)
	{
		return RoofMakers.Contains(inType);
	}

	inline static void SetRoofMakers(TMap<FString, TScriptInterface<IRoofMaker>> inRoofMakers)
	{
		RoofMakers = inRoofMakers;
	}
	
	static FBuildingMeshData CalculateMeshData(FBuildingPart* inBuildingPart, int inFirstSectionIndex, 
		UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial, FString inFlags = "");

	static void ConstructProceduralMesh(UProceduralMeshComponent* inProcMesh, FBuildingMeshData inMeshData);
	static void ConstructRuntimeMesh(URuntimeMeshComponent* inMeshComp, FBuildingMeshData inMeshData);
};
