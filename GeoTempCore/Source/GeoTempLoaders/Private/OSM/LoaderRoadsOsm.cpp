#include "Osm/LoaderRoadsOsm.h"

#include "OsmData.h"
#include "LoaderHelper.h"


void ULoaderRoadsOsm::SetOsmReader_Implementation(UOsmReader* inOsmReader)
{
	osmReader = inOsmReader;
}


TArray<FRoadSegment> GetRoadSegments(FOsmRoadNetwork inRoadNetwork)
{
	TArray<FRoadSegment> segments;
	for (auto osmSegmentPair : inRoadNetwork.Segments)
	{
		auto osmSegment = osmSegmentPair.Value;
		auto tags = osmSegment.Tags;
		auto highwayTag = tags.Find("highway");
		auto onewayTag = tags.Find("oneway");
		FRoadSegment segment;
		if (!highwayTag)
		{
			segment.Type = EHighwayType::Auto;
		}
		else
		{
			segment.Type = (highwayTag->Equals("footway") || highwayTag->Equals("pedestrian") || highwayTag->Equals("path")) ? EHighwayType::Footway : EHighwayType::Auto;
		}
		if (onewayTag)
		{
			segment.Oneway = onewayTag->Equals("yes");
		}

		segment.Lanes = ULoaderHelper::TryGetTag(tags, "lanes", ULoaderHelper::DEFAULT_LANES);
		segment.Width = ULoaderHelper::TryGetTag(tags, "widht", segment.Lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);

		segment.LanesForward = ULoaderHelper::TryGetTag(tags, "lanes:forward", segment.Lanes / 2);
		segment.LanesBackward = ULoaderHelper::TryGetTag(tags, "lanes:backward", segment.Lanes - segment.LanesForward);

		segment.AllPoints = osmSegment.Points;

		segments.Add(segment);
	}

	return segments;
}


FRoadNetwork ULoaderRoadsOsm::GetRoadNetwork_Implementation()
{
	TMap<int, FOsmRoadSegment> segments;
	TArray<FVector> intersectionPoints;
	TArray<FVector> trafficLights;
	for (auto wayData : osmReader->Ways)
	{
		OsmWay* way = wayData.Value;
		if (way->Tags.Contains("highway"))
		{

			auto contour = FContour();
			for (auto node : way->Nodes)
			{
				contour.Points.Add(node->Point);
				auto tagHighway = node->Tags.Find("highway");
				if (tagHighway && tagHighway->Equals("traffic_signals"))
				{
					trafficLights.Add(node->Point);
				}
			}
			TArray<FVector> intersections;
			auto cutContour = ULoaderHelper::CutContourByBounds(contour, osmReader->CutRect, intersections);
			ULoaderHelper::CutContourByBounds(contour, osmReader->BoundsRect, intersectionPoints);

			for (auto contourPiece : cutContour)
			{
				FOsmRoadSegment segment;

				TArray<FVector> points;
				for (auto point : contourPiece.Points)
				{
					points.Add(point);
				}

				segment.Points = points;
				segment.Tags = way->Tags;

				segments.Add(way->Id, segment);
			}
		}
	}
	auto roadNetwork = ULoaderHelper::ConstructRoadNetwork(GetRoadSegments(FOsmRoadNetwork{ segments }));
	roadNetwork.EntryPoints = intersectionPoints;
	for (auto& crossroad : roadNetwork.Crossroads)
	{
		int lightsIndex=-1;
		if (trafficLights.Find(crossroad.Value.Location, lightsIndex))
		{
			roadNetwork.TrafficLights.Add(crossroad.Key, trafficLights[lightsIndex]);
		}
	}
	return roadNetwork;
}
