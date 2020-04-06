#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OSMLoader.h"
#include "OSMBuildingLoader.h"
#include "OsmManager.h"
#include "MapDataLoader.generated.h"


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UMapDataLoader : public UActorComponent
{
	GENERATED_BODY()

	const float AreaMaxSizeDegrees = 0.1f;
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	UOsmBuildingLoader* BuildingsLoader;

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
};

