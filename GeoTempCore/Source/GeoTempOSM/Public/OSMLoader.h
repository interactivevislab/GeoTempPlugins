#include "CoreMinimal.h"
#include <GeoTempOSM\tinyxml2-master\tinyxml2.h>
#include "BuildingsData.h"
#include "Basics.h"
#include <unordered_map>
#include "Components/ActorComponent.h"
#include "OSMLoader.generated.h"



struct OSMNode
{
	long id;
	FVector Point;
	TMap<FString, FString> Tags;

	OSMNode(long id, double lon, double lat, float originLon, float originLat, float height = 0) : id(id)
	{
		Point = UGeoHelpers::GetLocalCoordinates(lon, lat, 0, FGeoCoords(ProjectionType::WGS84, originLon, originLat));
	}
};

struct OSMWay
{
	long id;
	std::vector<OSMNode*> Nodes;
	TMap<FString, FString> Tags;

	OSMWay(long id) : id(id) {};
};

struct OSMRelation
{
	long id;
    std::unordered_map<long, std::string> NodeRoles;
	std::unordered_map<long, std::string> WayRoles;
	std::unordered_map<long, std::string> RelRoles;
	std::unordered_map<long, OSMNode*> Nodes;
	std::unordered_map<long, OSMWay*> Ways;
	std::unordered_map<long, OSMRelation*> Relations;

	TMap<FString, FString> Tags;
	
	OSMRelation(long id) : id(id) {};
};

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GEOTEMPOSM_API UOSMReader : public UActorComponent
{
	GENERATED_BODY()

	UOSMReader() : UActorComponent()
	{		
	}

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
		float OriginLon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Geoprocessing")
		float OriginLat;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void ParseBuildings();
	
	
	TArray<FBuilding*> buildings;
	TArray<FBuildingPart*> parts;
	
	tinyxml2::XMLDocument XMLDocument;

	std::unordered_map<long, OSMNode*> Nodes;
	std::unordered_map<long, OSMWay*> Ways;
	std::unordered_map<long, OSMRelation*> Relations;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithXML(FString xmlString);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void InitWithFile(FString filename);

	void ReadData();

	static void initBuildingPart(const OSMWay* way, FBuildingPart* part);

	static void initBuildingPart(const OSMRelation* relation, FBuildingPart* part);
};

