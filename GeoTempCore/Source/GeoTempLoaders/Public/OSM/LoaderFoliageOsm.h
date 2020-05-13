#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderFolliage.h"
#include "OSMReader.h"
#include "GeometryData.h"
#include "LoaderFoliageOsm.generated.h"


class UOsmReader;

/**
* \class ULoaderFoliageOsm
* \brief A class to load foliage data for visualization.
*
* A class that is used to load foliage polygons for next visualization.
*/
UCLASS(Blueprintable)
class ULoaderFoliageOsm : public UObject, public IParserOsm, public IProviderFolliage
{
    GENERATED_BODY()

public:
    /** @name Implementation of IParserOsm. */
    ///@{
    virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
    ///@}

    
    /** @name Implementation of IProviderFolliage. */
    ///@{
    virtual TArray<FMultipolygonData> GetFolliage_Implementation() override;
    ///@}

private:
    /** An assigned UOsmReader to read foliage data from. */
    UPROPERTY()
    UOsmReader* osmReader;
};
