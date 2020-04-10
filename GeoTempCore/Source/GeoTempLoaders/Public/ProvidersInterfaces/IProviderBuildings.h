#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "BuildingsData.h"

#include "IProviderBuildings.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UProviderBuildings : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API IProviderBuildings
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FBuilding> GetBuildings();
};
