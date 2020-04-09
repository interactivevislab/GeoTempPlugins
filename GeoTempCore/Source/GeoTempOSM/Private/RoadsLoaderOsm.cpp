#include "RoadsLoaderOsm.h"

#include "OsmData.h"
#include "LoaderHelper.h"


void URoadsLoaderOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	OsmReader = inOsmReader;
}


TArray<FRoadSegment> GetRoadSegments(FOsmRoadNetwork inRoadNetwork)
{
	TArray<FRoadSegment> segments;
	for (auto osmSegmentPair : inRoadNetwork.Segments)
	{
		auto osmSegment = osmSegmentPair.Value;
		auto tags = osmSegment.Tags;

		FRoadSegment segment;
		segment.Type = EHighwayType::Auto;
		segment.Lanes = ULoaderHelper::TryGetTag(tags, "lanes", ULoaderHelper::DEFAULT_LANES);
		segment.Width = ULoaderHelper::TryGetTag(tags, "widht", segment.Lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);
		segment.AllPoints = osmSegment.Points;

		segments.Add(segment);
	}

	return segments;
}


FRoadNetwork URoadsLoaderOsm::GetRoadNetwork()
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

	return ULoaderHelper::ConstructRoadNetwork(GetRoadSegments(FOsmRoadNetwork{ segments }));
}
