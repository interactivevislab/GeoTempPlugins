#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Basics.h"
#include "BuildingsData.h"
#include "RoadsData.h"

#include "MapDataLoaderOsm.generated.h"


class UOsmReader;
class UOsmManager;
class UBuildingLoaderOsm;
class URoadsLoaderOsm;


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPLOADERS_API UMapDataLoaderOsm : public UActorComponent
{
	GENERATED_BODY()

	const float AreaMaxSizeDegrees = 0.5f;
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	UBuildingLoaderOsm* BuildingsLoader;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	URoadsLoaderOsm* RoadsLoader;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	UOsmReader* OsmReader;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	UOsmManager* OsmManager;

	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitManagers(bool inForceInit);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitLoaders(bool inForceInit);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees, bool inForceInitManagers = true, bool inForceInitLoaders = true);

	UFUNCTION()
	void OnOsmRequestCompleted(FString inXmlData);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Geoprocessing")
	bool IsDataReady;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	TArray<FBuilding> LoadedBuildings;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FRoadNetwork LoadedRoadNetwork;
};
