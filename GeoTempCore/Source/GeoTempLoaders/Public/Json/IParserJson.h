#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "JsonReader.h"

#include "IParserJson.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UParserJson : public UInterface
{
    GENERATED_BODY()
};


/**
* \class IParserJson
* \brief Interface for parsing JSON data.
*
* @see UJsonReader
*/
class GEOTEMPLOADERS_API IParserJson
{
    GENERATED_BODY()

public:

    /** Sets UJsonReader as data source. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetJsonReader(UJsonReader* inJsonReader);
};
