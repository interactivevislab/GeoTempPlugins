#pragma once

#include "CoreMinimal.h"

#include <vector>
#include <unordered_map>

#include "Basics.h"


struct OsmNode
{
	long Id;
	FVector Point;
	TMap<FString, FString> Tags;

	OsmNode(long id, double lon, double lat, FGeoCoords geoCoords, float height = 0);
};


struct OsmWay
{
	long Id;
	std::vector<OsmNode*> Nodes;
	TMap<FString, FString> Tags;

	OsmWay(long id);
};


struct OsmRelation
{
	long Id;

	std::unordered_map<long, std::string> NodeRoles;
	std::unordered_map<long, std::string> WayRoles;
	std::unordered_map<long, std::string> RelRoles;

	std::unordered_map<long, OsmNode*> Nodes;
	std::unordered_map<long, OsmWay*> Ways;
	std::unordered_map<long, OsmRelation*> Relations;

	TMap<FString, FString> Tags;

	OsmRelation(long id);
};