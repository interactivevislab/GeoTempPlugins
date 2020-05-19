#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "GeometryData.h"

#include "RuntimeMeshComponent.h"

#include "RoadBuilder.generated.h"


struct MeshSectionData;

struct RoadNetworkGeometry;

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
	static const int DefaultCapDensity;

	/** Maximum angle between triangles for arcs in road turns and crossroads. */
	static const float ArcsAngleStep;

	/** Material for road segments that be used in creating road network actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* RoadSegmentsMaterial;

	/** Material for crossroads that be used in creating road network actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* CrossroadsMaterial;

	/** Material for road curtains that be used in creating road network actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* CurtainsMaterial;

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
	static TArray<FVector2D> GetRoadCupsPointsDirections(int inCapDensity);

	/** Calculate points for edges cups. */
	UFUNCTION(BlueprintCallable)
	static TArray<FVector> GetCupsPointsOffsets(TArray<FVector2D> inPointsDirections, FVector inPerpendicularToLine, bool inIsReversedCup);

private:

	/**
	* \fn ConstructRoadMeshSection
	* \brief Add new mesh section in RuntimeMeshComponent.
	*
	* @param inRuntimeMesh				Target RuntimeMesh.
	* @param inNetworkGeometry			Road network geometry for mesh data calculation.
	* @param inSegmentsSectionIndex		Index of mesh section for road segments.
	* @param inCrossroadsSectionIndex	Index of mesh section for crossroads.
	* @param inSegmentsMaterial			Material for road segments' mesh section.
	* @param inCrossroadsMaterial		Material for crossroads' mesh section.
	* @param outCurtainsMeshData		Calculated data for roadsides' mesh section.
	*/
	void ConstructRoadMeshSection(URuntimeMeshComponent* inRuntimeMesh, RoadNetworkGeometry inNetworkGeometry,
		int inSegmentsSectionIndex, int inCrossroadsSectionIndex,
		UMaterialInstanceDynamic* inSegmentsMaterial, UMaterialInstanceDynamic* inCrossroadsMaterial,
		MeshSectionData& outCurtainsMeshData);

	/** Spawned RoadNetworkActor. */
	UPROPERTY()
	ARoadNetworkActor* roadNetworkActor;
};
