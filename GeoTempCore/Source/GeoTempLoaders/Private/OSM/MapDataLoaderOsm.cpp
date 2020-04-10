#include "OSM/MapDataLoaderOsm.h"

#include "HttpRequest.h"
#include "OsmReader.h"
#include "OsmManager.h"
#include "Osm/LoaderRoadsOsm.h"
#include "Osm/LoaderBuildingsOsm.h"
#include "Osm/LoaderFoliageOsm.h"



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
		BuildingsLoader = NewObject<ULoaderBuildingsOsm>();
		IParserOsm::Execute_SetOsmReader(BuildingsLoader, OsmReader);
	}
	if (!RoadsLoader || inForceInit)
	{
		RoadsLoader = NewObject<ULoaderRoadsOsm>();
		IParserOsm::Execute_SetOsmReader(RoadsLoader, OsmReader);
	}
	if (!FoliageLoader || inForceInit)
	{
		FoliageLoader = NewObject<ULoaderFoliageOsm>();
		IParserOsm::Execute_SetOsmReader(FoliageLoader, OsmReader);
	}
}


void UMapDataLoaderOsm::LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees, bool inForceInitManagers, bool inForceInitLoaders)
{	
	InitManagers(inForceInitManagers);

	InitLoaders(inForceInitLoaders);

	if (	inRightDegrees - inLeftDegrees > AreaMaxSizeDegrees 
		||	inLeftDegrees > inRightDegrees 
		||	inTopDegrees - inBottomDegrees > AreaMaxSizeDegrees 
		||	inTopDegrees < inBottomDegrees)
	{
		OnDataLoaded.Broadcast(false);
	}

	if (currentRequest != nullptr)
	{
		currentRequest->OnCompleted.RemoveAll(this);
	}
	currentRequest = OsmManager->GetOsmDataForBoundingBox(inLeftDegrees, inBottomDegrees, inRightDegrees, inTopDegrees);
	currentRequest->OnCompleted.AddDynamic(this, &UMapDataLoaderOsm::OnOsmRequestCompleted);
	currentRequest->StartRequest();
}


void UMapDataLoaderOsm::OnOsmRequestCompleted(FString inXmlData)
{		
	OsmReader->InitWithXML(inXmlData);

	LoadedBuildings			= IProviderBuildings::Execute_GetBuildings(BuildingsLoader);
	LoadedRoadNetwork		= IProviderRoads::Execute_GetRoadNetwork(RoadsLoader);
	LoadedFoliageContours	= IProviderFolliage::Execute_GetFolliage(FoliageLoader);
	OnDataLoaded.Broadcast(true);
}
