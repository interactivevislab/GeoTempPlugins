#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "BuildingsData.h"

#include "ILoaderBuildings.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderBuildings : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderBuildings
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FBuilding> GetBuildings();
};
