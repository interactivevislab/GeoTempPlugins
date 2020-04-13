#pragma once
#include "Components/ActorComponent.h"

#include "BuildingSpawner.generated.h"

class ABuildingActor;
class UMaterialInterface;
struct FBuilding;

/**
 * \class UBuildingSpawner
 * \brief main controller for buildings visualization and handling
 *
 * Controller component which responds for spawning buildings into scene and cleaning them if necessary
 * 
 */
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class GEOTEMPBUILDINGS_API UBuildingSpawner: public UActorComponent
{
	GENERATED_BODY()

public:

	UBuildingSpawner() {};

	/**
	 * \var Buildings
	 * \brief Array of Building Actors representing currently spawned buildings
	 * @see ABuildingActor
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Default")
	TArray<ABuildingActor*> Buildings;

	/**
	 * \fn SpawnBuildingActors
	 * \brief Process building structures and spawn building actors
	 * @param inBuildingData	array of building structures containing information about buildings
	 * @param inWallMaterial	material that will be assigned to building walls (and wall-type decorations)
	 * @param inRoofMaterial	material that will be assigned to building roofs (and roof-type decorations)
	 * @see FBuilding
	 */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void SpawnBuildingActors(const TArray<FBuilding>& inBuildingData, UMaterialInterface * inWallMaterial, UMaterialInterface * inRoofMaterial);

	/** Remove all currently spawned building actors */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void CleanBuildings();	
};
