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


/**
* \class IParserOsm
* \brief Interface for parsing OSM data.
*
* @see UOsmReader
*/
class GEOTEMPLOADERS_API IParserOsm
{
	GENERATED_BODY()

public:

	/** Sets UOsmReader as data source. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
};
