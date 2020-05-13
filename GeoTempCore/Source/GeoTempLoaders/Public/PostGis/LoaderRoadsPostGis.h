#pragma once

#include "CoreMinimal.h"

#include "IParserPostGis.h"
#include "ProvidersInterfaces/IProviderRoads.h"
#include "RoadsData.h"
#include "GeometryData.h"

#include "LoaderRoadsPostGis.generated.h"


/**
* \class ULoaderRoadsPostGis
* \brief Class for process roads data from PostGis.
*
* @see IParserPostGis, IProviderRoads
*/
UCLASS(Blueprintable)
class GEOTEMPLOADERS_API ULoaderRoadsPostGis : public UObject, public IParserPostGis, public IProviderRoads
{
    GENERATED_BODY()

public:

    /** @name Implementation of IParserPostGis */
    ///@{
    virtual void SetPostGisReader_Implementation(UPostGisReader* inPostGisReader) override;
    ///@}

    /** @name Implementation of IProviderRoads */
    ///@{
    virtual FRoadNetwork GetRoadNetwork_Implementation() override;
    ///@}

    /** Coordinates of the reference point in the scene space. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGeoCoords GeoCoodrs;

    /** Name of tag for road lines number. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString LanesTag;

    /** Name of tag for road width. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString WidthTag;

private:

    /** Inner data reader. */
    UPROPERTY()
    UPostGisReader* postGisReader;
};
