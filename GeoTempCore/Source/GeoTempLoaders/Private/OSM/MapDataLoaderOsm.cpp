#include "OSM/MapDataLoaderOsm.h"

#include "HttpRequest.h"
#include "OsmReader.h"
#include "OsmManager.h"
#include "Osm/LoaderRoadsOsm.h"
#include "Osm/LoaderBuildingsOsm.h"
#include "Osm/LoaderFoliageOsm.h"



void UMapDataLoaderOsm::Init()
{
	osmReader = NewObject<UOsmReader>(this);
	osmReader->GeoCoords = GeoCoords;

	osmManager = NewObject<UOsmManager>(this);
	osmManager->Init();

	buildingsLoader = NewObject<ULoaderBuildingsOsm>(this);
	IParserOsm::Execute_SetOsmReader(buildingsLoader, osmReader);

	roadsLoader = NewObject<ULoaderRoadsOsm>(this);
	IParserOsm::Execute_SetOsmReader(roadsLoader, osmReader);

	foliageLoader = NewObject<ULoaderFoliageOsm>();
	IParserOsm::Execute_SetOsmReader(foliageLoader, osmReader);

	isInitialized = true;
}


void UMapDataLoaderOsm::UpdateGeoCoords(FGeoCoords inGeoCoords)
{
	osmReader->GeoCoords = inGeoCoords;
}


void UMapDataLoaderOsm::LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees)
{	
	if (!isInitialized)
	{ 
		Init(); 
	}

	if (	inRightDegrees - inLeftDegrees > areaMaxSizeDegrees 
		||	inLeftDegrees > inRightDegrees 
		||	inTopDegrees - inBottomDegrees > areaMaxSizeDegrees 
		||	inTopDegrees < inBottomDegrees)
	{
		OnDataLoaded.Broadcast(false);
	}

	if (currentRequest != nullptr)
	{
		currentRequest->OnCompleted.RemoveAll(this);
	}
	currentRequest = osmManager->GetOsmDataForBoundingBox(inLeftDegrees, inBottomDegrees, inRightDegrees, inTopDegrees);
	currentRequest->OnCompleted.AddDynamic(this, &UMapDataLoaderOsm::OnOsmRequestCompleted);
	currentRequest->StartRequest();
}


void UMapDataLoaderOsm::OnOsmRequestCompleted(FString inXmlData)
{		
	osmReader->InitWithXML(inXmlData);

	LoadedBuildings			= IProviderBuildings::Execute_GetBuildings(buildingsLoader);
	LoadedRoadNetwork		= IProviderRoads::Execute_GetRoadNetwork(roadsLoader);
	LoadedFoliageContours	= IProviderFolliage::Execute_GetFolliage(foliageLoader);
	OnDataLoaded.Broadcast(true);
}


void UMapDataLoaderOsm::ClearLoadedData()
{
	LoadedBuildings.Empty();
	LoadedRoadNetwork = FRoadNetwork();
	LoadedFoliageContours.Empty();
}
