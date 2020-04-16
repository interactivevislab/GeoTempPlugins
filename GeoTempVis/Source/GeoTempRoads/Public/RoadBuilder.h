#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RuntimeMeshComponent.h"

#include "RoadBuilder.generated.h"


struct MeshSectionData;


/**
* \class URoadBuilder
* \brief Actor component, that can create road network actors.
*
* @see ARoadNetworkActor
*/
UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPROADS_API	URoadBuilder : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Material that be used in creating road network actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* RoadMaterial;

	/** Z-coordinate of highway surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoRoadZ;

	/** Z-coordinate of railroad surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RailRoadZ;

	/** Height of road down from Z-coordinate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoadHeight;

	/** Width of roadsides. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurtainsWidth;

	/** Stretch of road textures. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stretch = 1;

	/** Spawns ARoadNetworkActor based on road network structure. */
	UFUNCTION(BlueprintCallable)
	void SpawnRoadNetworkActor(FRoadNetwork inRoadNetwork);

private:

	/**
	* \fn ConstructRoadMeshSection
	* \brief Add new mesh section in RuntimeMeshComponent.
	*
	* @param inRuntimeMesh			Target RuntimeMesh.
	* @param inSegments				Array of road segments for mesh data calculation.
	* @param inSectionIndex			Index of target mesh section.
	* @param inMaterial				Material for mesh section.
	* @param outCurtainsMeshData	Calculated data for roadsides' mesh section.
	*/
	void ConstructRoadMeshSection(URuntimeMeshComponent* inRuntimeMesh, TArray<FRoadSegment> inSegments, 
		int inSectionIndex, UMaterialInstanceDynamic* inMaterial, MeshSectionData& outCurtainsMeshData);
};
