#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "PosgisData.h"

#include "RoadsLoaderPostGis.generated.h"



UCLASS(Blueprintable)
class GEOTEMPPOSTGIS_API URoadsLoaderPostGis : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRoadNetwork GetRoadNetwork(TArray<FPostGisBinaryEntity> inRoadData, FGeoCoords inGeoCoodrs);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString LanesTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString WidthTag;

private:

	static TArray<FRoadSegment> GetRoadSegments(FPostGisRoadNetwork inRoadNetwork);
};
