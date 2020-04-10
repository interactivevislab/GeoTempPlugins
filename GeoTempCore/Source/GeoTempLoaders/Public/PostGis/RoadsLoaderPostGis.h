#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"
#include "GeometryData.h"

#include "RoadsLoaderPostGis.generated.h"



UCLASS(Blueprintable)
class GEOTEMPLOADERS_API URoadsLoaderPostGis : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRoadNetwork GetRoadNetwork(TArray<FWkbEntity> inRoadData, FGeoCoords inGeoCoodrs);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString LanesTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString WidthTag;
};
