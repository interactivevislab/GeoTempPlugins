#include "OSM/MapDataLoader.h"
#include "HttpRequest.h"


void UMapDataLoader::InitManagers(bool inForceInit)
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


void UMapDataLoader::InitLoaders(bool inForceInit)
{
	if (!BuildingsLoader || inForceInit)
	{
		BuildingsLoader = NewObject<UOsmBuildingLoader>();
	}
}


void UMapDataLoader::LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees, bool inForceInitManagers, bool inForceInitLoaders)
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

	auto request = OsmManager->GetOsmDataForBoundingBox(inLeftDegrees, inBottomDegrees, inRightDegrees, inTopDegrees);	
	request->OnCompleted.AddDynamic(this, &UMapDataLoader::OnOsmRequestCompleted);
	request->StartRequest();
}

void UMapDataLoader::OnOsmRequestCompleted(FString inXmlData)
{
	OsmReader->InitWithXML(inXmlData);

	LoadedBuildings = BuildingsLoader->GetBuildings(OsmReader);

	IsDataReady = true;
}
