#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "ILoaderOsm.generated.h"


class UOsmReader;


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderOsm : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderOsm
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
};
