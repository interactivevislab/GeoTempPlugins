#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderRoads.h"
#include "RoadsData.h"

#include "LoaderRoadsOsm.generated.h"


class UOsmReader;


UCLASS(Blueprintable)
class GEOTEMPLOADERS_API ULoaderRoadsOsm : public UObject, public IParserOsm, public IProviderRoads
{
	GENERATED_BODY()

public:

	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
	virtual FRoadNetwork GetRoadNetwork_Implementation() override;

private:

	UOsmReader* osmReader;
};
