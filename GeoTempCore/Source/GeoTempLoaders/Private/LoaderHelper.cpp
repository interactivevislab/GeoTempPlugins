#include "LoaderHelper.h"


const int ULoaderHelper::DEFAULT_LANES = 2;
const float ULoaderHelper::DEFAULT_LANE_WIDTH = 3.5f;


FRoadNetwork ULoaderHelper::ConstructRoadNetwork(TArray<FRoadSegment> inRoadSegments)
{
	TMap<int, FRoadSegment> segments;
	TMap<int, FCrossroad>	crossroads;
	TMap<FVector, int>		crossroadIds;

	int nextSegmentId = 0;
	int nextCrossroadId = 0;

	for (auto segment : inRoadSegments)
	{
		auto pointStart = segment.AllPoints[0];
		auto pointEnd = segment.AllPoints[segment.AllPoints.Num() - 1];

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

		crossroadStart->Roads.Add(nextSegmentId, segment.EndCrossroadId);
		crossroadEnd->Roads.Add(nextSegmentId, segment.StartCrossroadId);

		segments.Add(nextSegmentId++, segment);
	}

	return FRoadNetwork{ segments, crossroads };
}


TArray<FContour> ULoaderHelper::FixRelationContours(TArray<FContour>& inUnclosedContours)
{
	TArray<FContour> closedContours = {};
	while (inUnclosedContours.Num() > 0)
	{
		if (closedContours.Num() == 0)
		{
			closedContours.Add(inUnclosedContours[0]);
			inUnclosedContours.RemoveAt(0);
			continue;
		}
		bool hasConnections = false;

		for (auto& contour : closedContours)
		{
			auto lastPointIndex = contour.Points.Num() - 1;
			int contourToRemove = -1;

			for (int i = 0; i < inUnclosedContours.Num(); i++)
			{
				if (contour.Points[lastPointIndex] == inUnclosedContours[i].Points[0])
				{
					for (int j = 1; j < inUnclosedContours[i].Points.Num(); j++)
					{
						contour.Points.Add(inUnclosedContours[i].Points[j]);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[0] == inUnclosedContours[i].Points.Last())
				{
					for (int j = inUnclosedContours[i].Points.Num() - 2; j >= 0; j--)
					{
						contour.Points.Insert(inUnclosedContours[i].Points[j], 0);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[0] == inUnclosedContours[i].Points[0])
				{
					for (int j = 1; j < inUnclosedContours[i].Points.Num(); j++)
					{
						contour.Points.Insert(inUnclosedContours[i].Points[j], 0);
					}
					contourToRemove = i;
					break;
				}

				if (contour.Points[lastPointIndex] == inUnclosedContours[i].Points.Last())
				{
					for (int j = inUnclosedContours[i].Points.Num() - 2; j >= 0; j--)
					{
						contour.Points.Add(inUnclosedContours[i].Points[j]);
					}
					contourToRemove = i;
					break;
				}
			}
			if (contourToRemove >= 0)
			{
				inUnclosedContours.RemoveAt(contourToRemove);
				hasConnections = true;
			}
			else
			{
				hasConnections = false;
			}
		}
		if (!hasConnections)
		{
			closedContours.Add(inUnclosedContours[0]);
			inUnclosedContours.RemoveAt(0);
			continue;
		}
	}
	for (auto& contour : closedContours)
	{
		if (!contour.IsClosed())
		{
			contour.FixClockwise();
		}
	}
	return closedContours;
}
