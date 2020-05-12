#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "Basics.h"
#include "BuildingsData.h"
#include "RoadsData.h"
#include "GeometryData.h"

#include "MapDataLoaderOsm.generated.h"


class UOsmReader;
class UOsmManager;
class ULoaderBuildingsOsm;
class ULoaderRoadsOsm;
class ULoaderFoliageOsm;
class ULoaderWaterOsm;
class UHttpRequest;


/**
* \class UMapDataLoaderOsm
* \brief Class for map data loading from OSM.
*/
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPLOADERS_API UMapDataLoaderOsm : public UActorComponent
{
	GENERATED_BODY()
	
public:

	/** Coordinates of the reference point in the scene space. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;

	/** Initialize inner managers and data loaders. */
	UFUNCTION(BlueprintCallable, Category = "Geoprocessing")
	void Init();

	/** Replace geocoordiantes in inner managers and data loaders. */
	UFUNCTION(BlueprintCallable, Category = "Geoprocessing")
	void UpdateGeoCoords(FGeoCoords inGeoCoords);

	/** Load all OSM data from map bounding box. */
	UFUNCTION(BlueprintCallable, Category = "Geoprocessing")
	void LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees);

	/** Clear all loaded OSM data. */
	UFUNCTION(BlueprintCallable, Category = "Geoprocessing")
	void ClearLoadedData();

	/** Load OSM data that was incompleted. */
	UFUNCTION(BlueprintCallable, Category = "Geoprocessing")
	void ReloadIncompleteData(FString inXmlData);
	
	/** Loaded and processed buildings data. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Geoprocessing")
	TArray<FBuilding> LoadedBuildings;

	/** Loaded and processed road network data. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Geoprocessing")
	FRoadNetwork LoadedRoadNetwork;

	/** Loaded and processed foliage data. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Geoprocessing")
	TArray<FMultipolygonData> LoadedFoliageContours;

	/** Loaded and processed water data. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Geoprocessing")
	TArray<FMultipolygonData> LoadedWaterContours;

	/** Delegate type for reporting completion of data loading. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataLoadedDelegate, bool, isSuccess);

	/** Delegate for reporting completion of data loading. */
	UPROPERTY(BlueprintAssignable, Category = "Geoprocessing")
	FOnDataLoadedDelegate OnDataLoaded;

private:

	/** Initialization flag. */
	bool isInitialized = false;

	/** Limit side size for loadable areas. */
	const float areaMaxSizeDegrees = 0.5f;

	/** Buffer for current HTTP request. */
	UHttpRequest* currentRequest = nullptr;

	/** Reader for OSM data. */
	UOsmReader* osmReader;

	/** Manager for OSM base. */
	UOsmManager* osmManager;

	/** Loader for buildings data. */
	ULoaderBuildingsOsm* buildingsLoader;

	/** Loader for road network data. */
	ULoaderRoadsOsm* roadsLoader;

	/** Loader for foliage data. */
	ULoaderFoliageOsm* foliageLoader;

	/** Loader for water data. */
	ULoaderWaterOsm* waterLoader;

	/** Limit side size for loadable areas. */
	int pendingRequests = 0;

	/** Limit side size for loadable areas. */
	TArray<int> pendingIds = {};

	/** Reads and processes data when HTTP request is completed. */
	UFUNCTION()
	void OnOsmRequestCompleted(FString inXmlData);
};

