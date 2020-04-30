#pragma once

#include "CoreMinimal.h"

#include "Basics.h"
#include "GeometryData.h"

#include "RuntimeMeshComponent.h"
#include "WaterActor.h"

#include "WaterBuilder.generated.h"


struct MeshSectionData;


/**
* \class UWaterBuilder
* \brief Actor component, that can create water actors.
*
* @see AWaterActor
*/
UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPWATER_API UWaterBuilder : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Material that be used in creating water actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* WaterMaterial;

	/** Z-coordinate of water surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float WaterZ;

	/** Spawns AWaterActor based on water polygon data. */
	UFUNCTION(BlueprintCallable)
		void SpawnWaterActor(const TArray<FMultipolygonData>& inPolygonData);

	/** Destroy spawned AWaterActor. */
	UFUNCTION(BlueprintCallable)
		void RemoveWaterActor();

	/**Add new mesh section in RuntimeMeshComponent. */
	UFUNCTION(BlueprintCallable)
		void AddWaterToMeshSingleSection(URuntimeMeshComponent * meshComponent, const TArray<FMultipolygonData>& inPolygonData, UMaterialInterface * material);

private:

	/** Spawned AWaterActor. */
	UPROPERTY()
	AWaterActor* waterActor;
};
