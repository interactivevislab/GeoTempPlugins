#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RuntimeMeshComponent.h"

#include "RoadBuilder.generated.h"


struct MeshSectionData
{
	TArray<FVector>				Vertices;
	TArray<int>					Indices;
	TArray<FVector>				Normals;
	TArray<FVector2D>			Uv0;
	TArray<FVector2D>			Uv1;
	TArray<FColor>				VertexColors;
	TArray<FRuntimeMeshTangent>	Tangents;
};


UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPROADS_API	URoadBuilder : public UActorComponent
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
	TMap<FString, RoadType> CoatingTags;

	UFUNCTION(BlueprintCallable)
	void SpawnRoadNetworkActor(FRoadNetwork inRoadNetwork);

private:

	void ConstructRoadMeshSection(URuntimeMeshComponent* inRuntimeMesh, TArray<FRoadSegment> inSegments, 
		int inSectionIndex, UMaterialInstanceDynamic* inMaterial, MeshSectionData& outCurtainsMeshData);

	UPROPERTY()
	TMap<int, UMaterialInstanceDynamic*> roadMaterials;

	const static int CURTAINS_MATERIAL_INDEX	= 0;
	const static int AUTO_MATERIAL_INDEX		= 1;
	const static int RAIL_MATERIAL_INDEX		= 2;
};
