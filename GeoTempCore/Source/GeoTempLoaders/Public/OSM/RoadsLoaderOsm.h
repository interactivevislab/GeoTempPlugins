#pragma once

#include "CoreMinimal.h"

#include "ILoaderOsm.h"
#include "RoadsData.h"

#include "RoadsLoaderOsm.generated.h"


class UOsmReader;


UCLASS(Blueprintable)
class GEOTEMPLOADERS_API URoadsLoaderOsm : public UObject, public ILoaderOsm
{
	GENERATED_BODY()

public:

	UOsmReader* OsmReader;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRoadNetwork GetRoadNetwork();
};
