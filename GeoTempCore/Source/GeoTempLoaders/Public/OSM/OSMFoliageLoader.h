#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "IReaderOsm.h"
#include "OSMLoader.h"
#include "PosgisData.h"
#include "OSMFoliageLoader.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UOsmFoliageLoader : public UObject, public IReaderOsm
{
	GENERATED_BODY()

public:

	UOsmReader* OsmReader;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetOsmReader(UOsmReader* inOsmReader);
	virtual void SetOsmReader_Implementation(UOsmReader* inOsmReader) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
		TArray<FPosgisContourData> GetFoliage();
};
