#pragma once

#include "CoreMinimal.h"

#include <GeoTempOSM\tinyxml2-master\tinyxml2.h>
#include "Components/ActorComponent.h"

#include "Basics.h"
#include "BuildingsData.h"
#include "OsmData.h"

#include "OsmReader.generated.h"


/**
* \class UOsmReader
* \brief Class for reading OSM data from xml.
*
* @see OsmNode, OsmWay, OsmRelation
*/
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPOSM_API UOsmReader : public UObject
{
	GENERATED_BODY()

	UOsmReader();

public:
	
	/** Coordinates of the reference point in the scene space. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
	FGeoCoords GeoCoords;

	/** Processed OSM nodes. */
	TMap<long, OsmNode*> Nodes;

	/** Processed OSM ways. */
	TMap<long, OsmWay*> Ways;

	/** Processed OSM relations. */
	TMap<long, OsmRelation*> Relations;

	/** Load data from XML string. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithXML(FString inXmlString);

	/** Load data from XML file. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithFile(FString inFilename);

private:

	/** Buffer for XML data. */
	tinyxml2::XMLDocument xmlDocument;

	/** Read OSM data from loaded XML document. */
	void ReadData();
};
