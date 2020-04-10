#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ILoaderOsm.h"
#include "OSMReader.h"
#include "GeometryData.h"
#include "FoliageLoaderOsm.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UFoliageLoaderOsm : public UObject, public ILoaderOsm
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UOsmReader* OsmReader;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FContourData> GetFoliage();
};
