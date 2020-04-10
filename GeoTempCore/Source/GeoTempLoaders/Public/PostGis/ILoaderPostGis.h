#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "PostgisReader.h"

#include "ILoaderPostGis.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API ULoaderPostGis : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API ILoaderPostGis
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetPostGisReader(APostgisReader* inPostGisReader);
};
