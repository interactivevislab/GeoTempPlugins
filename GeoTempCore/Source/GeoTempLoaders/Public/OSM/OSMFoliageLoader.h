#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "OSMLoader.h"
#include "PosgisData.h"
#include "OSMFoliageLoader.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UOsmFoliageLoader : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Default")
		TArray<FPosgisContourData> GetFoliage(UOsmReader* inSource, FGeoCoords inGeoCoords);
};
