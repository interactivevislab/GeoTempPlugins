#include "RoadsLoaderPostGis.h"

#include "LoaderHelper.h"


FRoadNetwork URoadsLoaderPostGis::GetRoadNetwork()
{
	FPostGisRoadNetwork stub;
	return ULoaderHelper::ConstructRoadNetwork(GetRoadSegments(stub));
}


TArray<FRoadSegment> URoadsLoaderPostGis::GetRoadSegments(FPostGisRoadNetwork inRoadNetwork)
{
	TArray<FRoadSegment> segments;
	for (auto postGisSegmentPair : inRoadNetwork.Segments)
	{
		auto postGisSegment = postGisSegmentPair.Value;

		FRoadSegment segment;
		segment.Type		= postGisSegment.Highway;
		segment.Width		= postGisSegment.Lanes * postGisSegment.LaneWidth;
		segment.Lanes		= postGisSegment.Lanes;
		segment.StartYear	= postGisSegment.YearStart;
		segment.EndYear		= postGisSegment.YearEnd;
		segment.Change		= postGisSegment.Change;
		segment.AllPoints	= postGisSegment.Line.AllPoints;

		segments.Add(segment);
	}

	return segments;
}
