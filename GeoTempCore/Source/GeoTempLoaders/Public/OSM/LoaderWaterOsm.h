#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderWater.h"
#include "OSMReader.h"
#include "GeometryData.h"
#include "LoaderWaterOsm.generated.h"


class UOsmReader;

/**
* \class ULoaderWaterOsm
* \brief A class to load water data for visualization.
*
* A class that is used to load water polygons for next visualization.
*/
UCLASS(Blueprintable)
class ULoaderWaterOsm : public UObject, public IParserOsm, public IProviderWater
{
	GENERATED_BODY()

public:
	/** @name Implementation of IParserOsm. */
	///@{
	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
	///@}


	/** @name Implementation of IProviderWater. */
	///@{
	virtual TArray<FMultipolygonData> GetWater_Implementation() override;
	///@}

	bool DataParsedSuccessfully = true;

	TSet<int> ErrorRelations = {};

private:
	/** An assigned UOsmReader to read water data from. */
	UPROPERTY()
		UOsmReader* osmReader;
};
