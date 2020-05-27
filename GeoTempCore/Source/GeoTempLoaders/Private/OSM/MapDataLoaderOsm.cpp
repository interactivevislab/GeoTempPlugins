#include "OSM/MapDataLoaderOsm.h"

#include "HttpRequest.h"
#include "OsmReader.h"
#include "OsmManager.h"
#include "Osm/LoaderRoadsOsm.h"
#include "Osm/LoaderBuildingsOsm.h"
#include "Osm/LoaderFoliageOsm.h"
#include "Osm/LoaderWaterOsm.h"



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

	waterLoader = NewObject<ULoaderWaterOsm>();
	IParserOsm::Execute_SetOsmReader(waterLoader, osmReader);

	isInitialized = true;
}


void UMapDataLoaderOsm::UpdateGeoCoords(FGeoCoords inGeoCoords)
{
	osmReader->GeoCoords = inGeoCoords;
}


void UMapDataLoaderOsm::SetBoundsRadius(int inBoundsRadius)
{
	if (osmReader)
	{
		auto Bounds = osmReader->BoundsRect;

		osmReader->CutRect = FVector4(Bounds.X - inBoundsRadius, Bounds.Y + inBoundsRadius, Bounds.Z - inBoundsRadius, Bounds.W + inBoundsRadius);
	}
}


void UMapDataLoaderOsm::LoadData(float inLeftDegrees, float inBottomDegrees, float inRightDegrees, float inTopDegrees)
{	
	
	if (!isInitialized)
	{ 
		Init(); 
	}
	ClearLoadedData();

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

	SetBoundsRadius(CutRadius);

	auto roadNetwork =				IProviderRoads::Execute_GetRoadNetwork(roadsLoader);
	LoadedRoadNetwork.Segments.Append(roadNetwork.Segments);
	LoadedRoadNetwork.Crossroads.Append(roadNetwork.Crossroads);
	LoadedRoadNetwork.EntryPoints.Append(roadNetwork.EntryPoints);

	LoadedBuildings			.Append(IProviderBuildings::Execute_GetBuildings(buildingsLoader)	);
	LoadedFoliageContours	.Append(IProviderFolliage::Execute_GetFolliage(foliageLoader)		);
	LoadedWaterContours		.Append(IProviderWater::Execute_GetWater(waterLoader)				);

	auto successfullParse = waterLoader->DataParsedSuccessfully && foliageLoader->DataParsedSuccessfully;

	if (!successfullParse)
	{
		osmReader->ClearReaderData();
		pendingRequests = waterLoader->ErrorRelations.Num()-1 + foliageLoader->ErrorRelations.Num() - 1;
		pendingIds = waterLoader->ErrorRelations.Array();
		pendingIds.Append(foliageLoader->ErrorRelations.Array());
		ReloadIncompleteData("");
	}
	else
	{
	OnDataLoaded.Broadcast(true);
	}
}

void UMapDataLoaderOsm::ReloadIncompleteData(FString inXmlData)
{
	osmReader->InitWithXML(inXmlData);
	if (currentRequest != nullptr)
	{
		currentRequest->OnCompleted.RemoveAll(this);
	}
	if (pendingRequests>=0)
	{
		currentRequest = osmManager->GetFullOsmDataForRelation(FString::FromInt(pendingIds[pendingRequests]));
		if (pendingRequests==0)
		{
			currentRequest->OnCompleted.AddDynamic(this, &UMapDataLoaderOsm::OnOsmRequestCompleted);
		}
		else
		{
			currentRequest->OnCompleted.AddDynamic(this, &UMapDataLoaderOsm::ReloadIncompleteData);
		}
		pendingRequests--;
		currentRequest->StartRequest();
		OnDataLoaded.Broadcast(false);
	}
}


void UMapDataLoaderOsm::ClearLoadedData()
{
	LoadedBuildings.Empty();
	LoadedRoadNetwork = FRoadNetwork();
	LoadedFoliageContours.Empty();
	LoadedWaterContours.Empty();

	osmReader->ClearReaderData();
}
