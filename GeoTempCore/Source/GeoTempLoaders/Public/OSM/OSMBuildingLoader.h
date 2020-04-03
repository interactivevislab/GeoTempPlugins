#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "OSMLoader.h"
#include "OSMBuildingLoader.generated.h"

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class UOsmBuildingLoader : public UObject
{
	GENERATED_BODY()
	
private:
	static void InitBuildingPart(const OsmWay* inWay, FBuildingPart* inPart);

	static void InitBuildingPart(const OsmRelation* inRelation, FBuildingPart* inPart);

public:
	UFUNCTION(BlueprintCallable, Category = "Default")
	TArray<FBuilding> GetBuildings(UOsmReader* source);
};

