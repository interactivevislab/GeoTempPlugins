#pragma once

#include "CoreMinimal.h"

#include "IParserOsm.h"
#include "ProvidersInterfaces/IProviderBuildings.h"

#include "LoaderBuildingsOsm.generated.h"


/**
* \class ULoaderBuildingsOsm
* \brief Class for process buildings data from OSM.
*
* @see IParserOsm, IProviderBuildings
*/
UCLASS(Blueprintable)
class GEOTEMPLOADERS_API ULoaderBuildingsOsm : public UObject, public IParserOsm, public IProviderBuildings
{
    GENERATED_BODY()

public:

    /** @name Implementation of IParserOsm */
    ///@{
    virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;
    ///@}

    /** @name Implementation of IProviderBuildings */
    ///@{
    virtual TArray<FBuilding> GetBuildings_Implementation() override;
    ///@}

private:

    /** Inner data reader. */
    UPROPERTY()
    UOsmReader* osmReader;
};
