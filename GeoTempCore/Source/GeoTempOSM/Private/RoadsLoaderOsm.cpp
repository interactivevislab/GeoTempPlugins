#include "RoadsLoaderOsm.h"

#include "OsmData.h"


void URoadsLoaderOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	OsmReader = inOsmReader;
}


FOsmRoadNetwork URoadsLoaderOsm::GetRoadNetwork(FGeoCoords inGeoCoords)
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
