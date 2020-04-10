#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "OsmReader.h"

#include "IParserOsm.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UParserOsm : public UInterface
{
	GENERATED_BODY()
};


class GEOTEMPLOADERS_API IParserOsm
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
};
