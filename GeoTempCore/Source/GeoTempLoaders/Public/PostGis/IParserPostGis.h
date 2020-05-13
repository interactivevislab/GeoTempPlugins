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


/**
* \class IParserPostGis
* \brief Interface for parsing PostGis data.
*
* @see UPostGisReader
*/
class GEOTEMPLOADERS_API IParserPostGis
{
    GENERATED_BODY()

public:

    /** Sets UPostGisReader as data source. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetPostGisReader(UPostGisReader* inPostGisReader);
};
