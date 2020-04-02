#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RuntimeMeshComponent.h"

#include "RoadBuilder.generated.h"


struct MeshSectionData
{
	TArray<FVector>				vertices;
	TArray<int>					indices;
	TArray<FVector>				normales;
	TArray<FVector2D>			uv0;
	TArray<FVector2D>			uv1;
	TArray<FColor>				vertexColors;
	TArray<FRuntimeMeshTangent>	tangents;
};


UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPROADS_API	URoadBuilder : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* RoadMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoRoadZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RailRoadZ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoadHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurtainsWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stretch = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CoatingChangeYearStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CoatingChangeYearEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, RoadType> CoatingTags;

	UFUNCTION(BlueprintCallable)
	void AddRoadNetworkToMesh(FRoadNetwork inRoadNetwork);

	UFUNCTION(BlueprintCallable)
	void SetYear(int inYear);

	UFUNCTION(BlueprintCallable)
	void UpdateLandscapeData(FVector4 inRect);

private:

	inline void ConstructRoadMeshSection(TArray<FRoadSegment> inSegments, int inSectionIndex,
		UMaterialInstanceDynamic* inMaterial, MeshSectionData& outCurtainsMeshData);

	UPROPERTY()
	TMap<int, UMaterialInstanceDynamic*> roadMaterials;

	const static int CURTAINS_MATERIAL_INDEX	= 0;
	const static int RAIL_MATERIAL_INDEX		= 1;

	struct CoatingChangeData
	{
		int MaterialIndex;
		RoadType TargetCoating;
		int ChangeYear;
	};

	TArray<CoatingChangeData> coatingChangeDatas;
};
