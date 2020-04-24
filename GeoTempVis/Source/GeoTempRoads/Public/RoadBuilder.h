#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "GeometryData.h"

#include "RuntimeMeshComponent.h"

#include "RoadBuilder.generated.h"


struct MeshSectionData;


class ARoadNetworkActor;


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

	/** Number of points used to create round road edges. */
	static const int capDensity = 8;

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

	/** Destroy spawned RoadNetworkActor. */
	UFUNCTION(BlueprintCallable)
	void RemoveRoadNetworkActor();

	/** Calculate perpendicular vector represented with start and end points. */
	UFUNCTION(BlueprintCallable)
	static FVector CalculatePerpendicularToLine(FVector inStartPoint, FVector inEndPoint);

	/** Create rectangle from a line represented with specified thickness. */
	UFUNCTION(BlueprintCallable)
	static TArray<FVector> ConvertLineToRect(FVector inStartPoint, FVector inEndPoint, FVector inPerpendicularToLine);

	/** Calculate direction vectors for road segment round edges. Used in UVs and calculating points for edges cups. */
	UFUNCTION(BlueprintCallable)
	static TArray<FVector2D> GetRoadCupsPointsDirections();

	/** Calculate points for edges cups. */
	UFUNCTION(BlueprintCallable)
	static TArray<FVector> GetCupsPointsOffsets(TArray<FVector2D> inPointsDirections, FVector inPerpendicularToLine, bool inIsReversedCup);

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

	/** Spawned RoadNetworkActor. */
	ARoadNetworkActor* roadNetworkActor = nullptr;
};
