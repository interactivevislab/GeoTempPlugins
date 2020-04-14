#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "GeometryData.h"

#include "IProviderFolliage.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UProviderFolliage : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API IProviderFolliage
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FMultipolygonData> GetFolliage();
};
