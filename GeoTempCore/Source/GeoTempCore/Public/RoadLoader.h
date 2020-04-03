#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RoadLoader.generated.h"


UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPCORE_API URoadLoader : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork ProcessRoadNetwork(FPostGisRoadNetwork inApiRoadNetwork);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear);
};
