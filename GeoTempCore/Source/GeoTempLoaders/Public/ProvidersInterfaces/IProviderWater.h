#pragma once

#include "CoreMinimal.h"

#include "UObject/Interface.h"

#include "GeometryData.h"

#include "IProviderWater.generated.h"


UINTERFACE(BlueprintType)
class GEOTEMPLOADERS_API UProviderWater : public UInterface
{
	GENERATED_BODY()
};


/**
* \class IProviderFolliage
* \brief Interface for providing folliage data.
*
* @see FContourData
*/
class GEOTEMPLOADERS_API IProviderWater
{
	GENERATED_BODY()

public:

	/** Provide folliage data. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		TArray<FMultipolygonData> GetWater();
};
