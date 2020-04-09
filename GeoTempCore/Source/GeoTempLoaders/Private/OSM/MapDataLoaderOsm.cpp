#include "OSM/MapDataLoaderOsm.h"

#include "HttpRequest.h"
#include "OsmReader.h"
#include "OsmManager.h"
#include "Osm/RoadsLoaderOsm.h"
#include "Osm/BuildingLoaderOsm.h"



void UMapDataLoaderOsm::InitManagers(bool inForceInit)
{
	if (!OsmReader || inForceInit) 
	{
		OsmReader = NewObject<UOsmReader>();
		OsmReader->GeoCoords = GeoCoords;
	}
	if (!OsmManager || inForceInit) 
	{
		OsmManager = NewObject<UOsmManager>();
		OsmManager->Init();
	}
}


void UMapDataLoaderOsm::InitLoaders(bool inForceInit)
{
	if (!BuildingsLoader || inForceInit)
	{
		BuildingsLoader = NewObject<UBuildingLoaderOsm>();
	}
	if (!RoadsLoader || inForceInit)
	{
		RoadsLoader = NewObject<URoadsLoaderOsm>();
		RoadsLoader->SetOsmReader(OsmReader);
	}
}


void UMapDataLoaderOsm::LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees, bool inForceInitManagers, bool inForceInitLoaders)
{	
	InitManagers(inForceInitManagers);

	InitLoaders(inForceInitLoaders);

	if (inRightDegrees - inLeftDegrees > AreaMaxSizeDegrees || inLeftDegrees > inRightDegrees)
	{
		return;
	}
	if (inTopDegrees - inBottomDegrees> AreaMaxSizeDegrees || inTopDegrees < inBottomDegrees)
	{
		return;
	}
	IsDataReady = false;
	UHttpRequest* request = OsmManager->GetOsmDataForBoundingBox(inLeftDegrees, inBottomDegrees, inRightDegrees, inTopDegrees);
	request->OnCompleted.AddDynamic(this, &UMapDataLoaderOsm::OnOsmRequestCompleted);
	request->StartRequest();
}


void UMapDataLoaderOsm::OnOsmRequestCompleted(FString inXmlData)
{		
	OsmReader->InitWithXML(inXmlData);

	LoadedBuildings		= BuildingsLoader->GetBuildings(OsmReader);
	LoadedRoadNetwork	= RoadsLoader->GetRoadNetwork();

	IsDataReady = true;
}
