#include "RoadHelper.h"


FRoadNetwork URoadHelper::ProcessRoadNetwork(FPostGisRoadNetwork inApiRoadNetwork)
{
	TMap<int, FRoadSegment> segments;
	TMap<int, FCrossroad>	crossroads;
	TMap<FVector, int>		crossroadIds;

	int nextSegmentId = 0;
	int nextCrossroadId = 0;

	for (auto apiSegmentPair : inApiRoadNetwork.Segments)
	{
		auto apiSegment = apiSegmentPair.Value;

		FRoadSegment segment;
		segment.Type		= apiSegment.Highway;
		segment.Width		= apiSegment.Lanes * apiSegment.LaneWidth;
		segment.Lanes		= apiSegment.Lanes;
		segment.StartYear	= apiSegment.YearStart;
		segment.EndYear		= apiSegment.YearEnd;
		segment.Change		= apiSegment.Change;

		auto pointStart = apiSegment.Line.Start;
		auto pointEnd	= apiSegment.Line.End;

		FCrossroad* crossroadStart;
		FCrossroad* crossroadEnd;

		auto ptr = crossroadIds.Find(pointStart);
		if (ptr == nullptr)
		{
			crossroadIds.Add(pointStart, nextCrossroadId);
			crossroadStart = &(crossroads.Add(nextCrossroadId, FCrossroad{ pointStart }));
			segment.StartCrossroadId = nextCrossroadId++;
		}
		else
		{
			segment.StartCrossroadId = *ptr;
			crossroadStart = crossroads.Find(*ptr);
		}

		ptr = crossroadIds.Find(pointEnd);
		if (ptr == nullptr)
		{
			crossroadIds.Add(pointEnd, nextCrossroadId);
			crossroadEnd = &(crossroads.Add(nextCrossroadId, FCrossroad{ pointEnd }));
			segment.EndCrossroadId = nextCrossroadId++;
		}
		else
		{
			segment.EndCrossroadId = *ptr;
			crossroadEnd = crossroads.Find(*ptr);
		}

		segment.AllPoints = apiSegmentPair.Value.Line.AllPoints;

		crossroadStart->Roads.Add(nextSegmentId, segment.EndCrossroadId);
		crossroadEnd->Roads.Add(nextSegmentId, segment.StartCrossroadId);

		segments.Add(nextSegmentId++, segment);
	}

	return FRoadNetwork{ segments, crossroads };
}


FRoadNetwork URoadHelper::GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear)
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