#pragma once
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "BuildingsData.h"
#include "BuildingActor.generated.h"



class UBuildingPartComponent;

/** Actor class of a building in city scene*/
UCLASS()
class GEOTEMPBUILDINGS_API ABuildingActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildingActor();

	/** List of Outer contours of building footprint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Outer;

	/** List of inner contours (a.k.a. holes) of building footprint */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Inner;

	/** Dictionary of tags applied to the building on load. Can include address, material and other useful data*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tags")
	TMap<FString, FString> BuildingTags;

	/** Material for walls (and wall-type decorations) of the building */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	UMaterialInterface* WallMaterial;

	/** Material for roofs (and roof-type decorations) of the building */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	UMaterialInterface* RoofMaterial;

	/** Id of this building */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	int Id;

	/**
	 * \var Parts
	 * \brief List of all parts this building consists of
	 * 
	 * @see UBuildingPartComponent	 
	 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TArray<UBuildingPartComponent*> Parts;

	/**
	 * Building structure contains parameters of this building
	 */
	FBuilding Building;

	/** Construction initialize function */
	void OnConstruction(const FTransform& Transform) override;

	/** \fn Initialize
	 * \brief Initialize with building data
	 * 
	 * @param inBuilding data to initialize with
	 * @param inInitPartsImmideately should this function also call mesh initializing for created parts
	 * @see FBuilding
	 */
	virtual void Initialize(const FBuilding& inBuilding, bool inInitPartsImmideately = true);

	/** Reset initialization parameters if them where changed from last initialization */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void ReInitialize();
};
