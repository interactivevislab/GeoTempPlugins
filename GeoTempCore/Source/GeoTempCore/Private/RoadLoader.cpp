#include "RoadLoader.h"


FRoadNetwork URoadLoader::ProcessPostGisRoadNetwork(FPostGisRoadNetwork inRoadNetwork)
{
	return ProcessRoadNetwork(inRoadNetwork);
}


FRoadNetwork URoadLoader::ProcessOsmRoadNetwork(FOsmRoadNetwork inRoadNetwork)
{
	return ProcessRoadNetwork(inRoadNetwork);
}


FRoadNetwork URoadLoader::GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear)
{
	TMap<int, FRoadSegment>	segments;
	TMap<int, FCrossroad>	crossroads;
	TSet<int>				crossroadsIds;

	for (auto segmentData : inFullRoadNetwork.Segments)
	{
		auto segment = segmentData.Value;
		if ((segment.StartYear <= inYear) && (segment.EndYear > inYear))
		{
			segments.Add(segmentData);
			crossroadsIds.Add(segment.StartCrossroadId);
			crossroadsIds.Add(segment.EndCrossroadId);
		}
	}

	for (auto crossroadId : crossroadsIds)
	{
		auto crossroad = *(inFullRoadNetwork.Crossroads.Find(crossroadId));
		crossroads.Add(crossroadId, crossroad);
	}

	return FRoadNetwork{ segments, crossroads };
}