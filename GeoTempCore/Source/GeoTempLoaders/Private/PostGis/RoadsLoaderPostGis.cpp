#include "PostGis/RoadsLoaderPostGis.h"

#include "LoaderHelper.h"



TArray<FRoadSegment> GetRoadSegments(FPostGisRoadNetwork inRoadNetwork)
{
	TArray<FRoadSegment> segments;
	for (auto postGisSegmentPair : inRoadNetwork.Segments)
	{
		auto postGisSegment = postGisSegmentPair.Value;

		FRoadSegment segment;
		segment.Type = postGisSegment.Highway;
		segment.Width = postGisSegment.Lanes * postGisSegment.LaneWidth;
		segment.Lanes = postGisSegment.Lanes;
		segment.StartYear = postGisSegment.YearStart;
		segment.EndYear = postGisSegment.YearEnd;
		segment.Change = postGisSegment.Change;
		segment.AllPoints = postGisSegment.Line.AllPoints;

		segments.Add(segment);
	}

	return segments;
}


FRoadNetwork URoadsLoaderPostGis::GetRoadNetwork(TArray<FWkbEntity> inRoadData, FGeoCoords inGeoCoodrs)
{
	FPostGisRoadNetwork postGisRoadNetwork;
	int segmentId = 0;
	for (auto segmentData : inRoadData)
	{
		int offset = 4;
		auto data = segmentData.Geometry.GetData();
		uint32 primitiveCount = *((uint32*)(data + offset * sizeof(char)));
		offset += 4;

		auto tags = segmentData.Tags;

		for (uint32 primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++)
		{
			auto presenceTag = tags.Find("Presence");
			//ignoring roads with Presence = 0
			if ((presenceTag != nullptr) && (!presenceTag->IsEmpty()) && (FCString::Atoi(**presenceTag) == 0))
			{
				continue;
			}	

			auto points = FContourData::BinaryParseCurve(segmentData.Geometry.GetData(), offset, 
				inGeoCoodrs, true, 0);

			FPostGisRoadSegment segment;
			segment.Line = {
				points[0],
				points[points.Num() - 1],
				points
			};

			auto lanes	= ULoaderHelper::TryGetTag(tags, LanesTag, ULoaderHelper::DEFAULT_LANES);
			auto width	= ULoaderHelper::TryGetTag(tags, WidthTag, lanes * ULoaderHelper::DEFAULT_LANE_WIDTH);

			segment.Highway		= EHighwayType::Auto;
			segment.Lanes		= lanes;
			segment.LaneWidth	= width;

			postGisRoadNetwork.Segments.Add(segmentId++, segment);
		}
	}

	return ULoaderHelper::ConstructRoadNetwork(GetRoadSegments(postGisRoadNetwork));
}
