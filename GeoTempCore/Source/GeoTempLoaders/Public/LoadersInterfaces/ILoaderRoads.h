#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "RoadsData.h"

#include "ILoaderRoads.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderRoads : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderRoads
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FRoadNetwork GetRoadNetwork();
};
