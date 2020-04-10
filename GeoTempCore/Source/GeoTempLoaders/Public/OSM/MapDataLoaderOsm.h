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
class UBuildingLoaderOsm;
class URoadsLoaderOsm;
class UFoliageLoaderOsm;
class UHttpRequest;


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPLOADERS_API UMapDataLoaderOsm : public UActorComponent
{
	GENERATED_BODY()

	const float AreaMaxSizeDegrees = 0.5f;

	UHttpRequest* currentRequest = nullptr;
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;
	
	UPROPERTY(BlueprintReadWrite, Category = "Geoprocessing")
	UBuildingLoaderOsm* BuildingsLoader;

	UPROPERTY(BlueprintReadWrite, Category = "Geoprocessing")
	URoadsLoaderOsm* RoadsLoader;

	UPROPERTY(BlueprintReadWrite, Category = "Geoprocessing")
	UFoliageLoaderOsm* FoliageLoader;

	UPROPERTY(BlueprintReadWrite, Category = "Geoprocessing")
	UOsmReader* OsmReader;

	UPROPERTY(BlueprintReadWrite, Category = "Geoprocessing")
	UOsmManager* OsmManager;

	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitManagers(bool inForceInit);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void InitLoaders(bool inForceInit);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees, bool inForceInitManagers = true, bool inForceInitLoaders = true);

	UFUNCTION()
	void OnOsmRequestCompleted(FString inXmlData);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	TArray<FBuilding> LoadedBuildings;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FRoadNetwork LoadedRoadNetwork;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	TArray<FContourData> LoadedFoliageContours;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataLoadedDelegate, bool, isSuccess);

	UPROPERTY(BlueprintAssignable, Category = "Geoprocessing")
	FOnDataLoadedDelegate OnDataLoaded;
};

