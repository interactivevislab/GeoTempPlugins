#include "RoadsLoaderOsm.h"

#include "OsmData.h"


void URoadsLoaderOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	OsmReader = inOsmReader;
}


FOsmRoadNetwork URoadsLoaderOsm::GetRoadNetwork()
{
	TMap<int, FOsmRoadSegment> segments;
	for (auto wayData : OsmReader->Ways)
	{
		OsmWay* way = wayData.second;
		if (way->Tags.Contains("highway"))
		{
			FOsmRoadSegment segment;

			TArray<FVector> points;
			for (auto node : way->Nodes)
			{
				points.Add(node->Point);
			}

			segment.Points = points;
			segment.Tags = way->Tags;

			segments.Add(way->Id, segment);
		}
	}

	return FOsmRoadNetwork{ segments };
}


TArray<FRoadSegment> URoadsLoaderOsm::GetRoadSegments(FOsmRoadNetwork inRoadNetwork)
{
	TArray<FRoadSegment> segments;
	for (auto osmSegmentPair : inRoadNetwork.Segments)
	{
		auto osmSegment = osmSegmentPair.Value;

		//TODO: add tag processing for non-constannt values
		FRoadSegment segment;
		segment.Type		= EHighwayType::Auto;
		segment.Width		= 7;
		segment.Lanes		= 2;
		segment.StartYear	= 0;
		segment.EndYear		= 3000;
		segment.Change		= "";
		segment.AllPoints	= osmSegment.Points;

		segments.Add(segment);
	}

	return segments;
}
