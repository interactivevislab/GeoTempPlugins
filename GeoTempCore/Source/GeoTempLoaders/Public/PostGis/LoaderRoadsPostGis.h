#pragma once

#include "CoreMinimal.h"

#include "IParserPostGis.h"
#include "ProvidersInterfaces/IProviderRoads.h"
#include "RoadsData.h"
#include "GeometryData.h"

#include "LoaderRoadsPostGis.generated.h"



UCLASS(Blueprintable)
class GEOTEMPLOADERS_API ULoaderRoadsPostGis : public UObject, public IParserPostGis, public IProviderRoads
{
	GENERATED_BODY()

public:

	virtual void SetPostGisReader_Implementation(UPostGisReader* inPostGisReader) override;
	virtual FRoadNetwork GetRoadNetwork_Implementation() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGeoCoords GeoCoodrs;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString LanesTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString WidthTag;

private:

	UPostGisReader* postGisReader;
};
