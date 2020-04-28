#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderRoads.h"
#include "RoadsData.h"

#include "LoaderRoadsOsm.generated.h"


class UOsmReader;


/**
* \class ULoaderRoadsOsm
* \brief Class for process roads data from OSM.
*
* @see IParserOsm, IProviderRoads
*/
UCLASS(Blueprintable)
class GEOTEMPLOADERS_API ULoaderRoadsOsm : public UObject, public IParserOsm, public IProviderRoads
{
	GENERATED_BODY()

public:

	/** @name Implementation of IParserOsm */
	///@{
	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
	///@}

	/** @name Implementation of IProviderRoads */
	///@{
	virtual FRoadNetwork GetRoadNetwork_Implementation() override;
	///@}

private:

	/** Inner data reader. */
	UPROPERTY()
	UOsmReader* osmReader;
};
