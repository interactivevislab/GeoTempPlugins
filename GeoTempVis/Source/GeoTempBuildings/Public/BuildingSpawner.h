#pragma once
#include "Components/ActorComponent.h"


#include "BuildingSpawner.generated.h"

class ABuildingActor;
class UMaterialInterface;
struct FBuilding;

UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class GEOTEMPBUILDINGS_API UBuildingSpawner: public UActorComponent
{
	GENERATED_BODY()

public:

	UBuildingSpawner() {};

	UPROPERTY(BlueprintReadWrite, Category = "Default")
	TArray<ABuildingActor*> Buildings;

	UFUNCTION(BlueprintCallable, Category = "Default")
	void SpawnBuildingActors(const TArray<FBuilding>& inBuildingData, UMaterialInterface * inWallMaterial, UMaterialInterface * inRoofMaterial);


	UFUNCTION(BlueprintCallable, Category = "Default")
	void CleanBuildings();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString IdTag = "Id";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString FloorsTag = "Floors";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString MinFloorsTag = "MinFloors";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString HeightTag = "Height";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString MinHeightTag = "MinHeight";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString BuildStartTag = "BegAppear";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString BuildEndTag = "EndAppear";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString DemolishStartTag = "DatDemol";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString RoofTypeTag = "RoofType";
	
};
