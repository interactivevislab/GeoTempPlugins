#pragma once

#include "CoreMinimal.h"

#include <GeoTempOSM\tinyxml2-master\tinyxml2.h>
#include <unordered_map>
#include "Components/ActorComponent.h"

#include "Basics.h"
#include "BuildingsData.h"
#include "OsmData.h"

#include "OSMLoader.generated.h"


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPOSM_API UOsmReader : public UObject
{
	GENERATED_BODY()

	UOsmReader();

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;

	tinyxml2::XMLDocument XmlDocument;

	std::unordered_map<long, OsmNode*> Nodes;
	std::unordered_map<long, OsmWay*> Ways;
	std::unordered_map<long, OsmRelation*> Relations;


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithXML(FString inXmlString);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithFile(FString inFilename);

	void ReadData();


};
