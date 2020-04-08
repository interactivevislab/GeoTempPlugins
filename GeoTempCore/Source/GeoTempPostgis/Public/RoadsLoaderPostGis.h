#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RoadsLoaderPostGis.generated.h"



UCLASS(Blueprintable)
class GEOTEMPPOSTGIS_API URoadsLoaderPostGis : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<FRoadSegment> GetRoadSegments(FPostGisRoadNetwork inRoadNetwork);
};
