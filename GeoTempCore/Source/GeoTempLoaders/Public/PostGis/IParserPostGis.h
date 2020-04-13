#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "PostgisReader.h"

#include "IParserPostGis.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UParserPostGis : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API IParserPostGis
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetPostGisReader(UPostGisReader* inPostGisReader);
};
