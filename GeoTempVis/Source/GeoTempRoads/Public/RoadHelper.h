#pragma once

#include "CoreMinimal.h"

#include "RoadsData.h"

#include "RoadHelper.generated.h"


UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
class GEOTEMPROADS_API URoadHelper : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork ProcessRoadNetwork(FPostGisRoadNetwork inApiRoadNetwork);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRoadNetwork GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear);
};
