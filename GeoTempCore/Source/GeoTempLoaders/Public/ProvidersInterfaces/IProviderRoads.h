#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "RoadsData.h"

#include "IProviderRoads.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UProviderRoads : public UInterface
{
    GENERATED_BODY()
};


/**
* \class IProviderRoads
* \brief Interface for providing roads data.
*
* @see FRoadNetwork
*/
class GEOTEMPLOADERS_API IProviderRoads
{
    GENERATED_BODY()

public:

    /** Provide roads data. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FRoadNetwork GetRoadNetwork();
};
