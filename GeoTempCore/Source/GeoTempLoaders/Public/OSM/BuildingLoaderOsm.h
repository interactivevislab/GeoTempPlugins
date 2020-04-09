#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "OsmReader.h"
#include "BuildingLoaderOsm.generated.h"


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPLOADERS_API UBuildingLoaderOsm : public UObject
{
	GENERATED_BODY()
private:	
	
	static void InitBuildingPart(const OsmWay* inWay, FBuildingPart& outPart);

	static void InitBuildingPart(const OsmRelation* inRelation, FBuildingPart& outPart);

public:

	static FString FLOORS_TAG_STRING;
	static FString HEIGHT_TAG_STRING;
	static FString MIN_FLOORS_TAG_STRING;
	static FString MIN_HEIGHT_TAG_STRING;
	
	UFUNCTION(BlueprintCallable, Category = "Default")
	TArray<FBuilding> GetBuildings(UOsmReader* inSource);
};
