#pragma once

#include "CoreMinimal.h"

#include "Basics.h"


/** OSM node. */
struct OsmNode
{
	long Id;
	FVector Point;
	TMap<FString, FString> Tags;

	OsmNode(long id, double lon, double lat, FGeoCoords geoCoords, float height = 0);
};


/** OSM way. */
struct OsmWay
{
	long Id;
	TArray<OsmNode*> Nodes;
	TMap<FString, FString> Tags;

	OsmWay(long id);
};


/** OSM relation. */
struct OsmRelation
{
	long Id;

	TMap<long, FString> NodeRoles;
	TMap<long, FString> WayRoles;
	TMap<long, FString> RelRoles;

	TMap<long, OsmNode*> Nodes;
	TMap<long, OsmWay*> Ways;
	TMap<long, OsmRelation*> Relations;

	TMap<FString, FString> Tags;

	OsmRelation(long id);
};
