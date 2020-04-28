#include "PostGis/LoaderRoadsPostGis.h"

#include "LoaderHelper.h"


void ULoaderRoadsPostGis::SetPostGisReader_Implementation(UPostGisReader* inPostGisReader)
{
	postGisReader = inPostGisReader;
}


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
		segment.AllPoints = postGisSegment.Line.AllPoints;

		segments.Add(segment);
	}

	return segments;
}


FRoadNetwork ULoaderRoadsPostGis::GetRoadNetwork_Implementation()
{
	FPostGisRoadNetwork postGisRoadNetwork;
	int segmentId = 0;
	for (auto segmentData : postGisReader->RawQueryResult)
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

			auto points = FMultipolygonData::BinaryParseCurve(segmentData.Geometry.GetData(), offset, GeoCoodrs, true, 0);

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
