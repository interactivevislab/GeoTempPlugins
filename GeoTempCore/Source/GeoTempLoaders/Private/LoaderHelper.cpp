#include "LoaderHelper.h"


const int ULoaderHelper::DEFAULT_LANES = 1;
const float ULoaderHelper::DEFAULT_LANE_WIDTH = 3.5f;


FRoadNetwork ULoaderHelper::ConstructRoadNetwork(const TArray<FRoadSegment>& inRoadSegments)
{
	TMap<int, FRoadSegment> segments;
	TMap<int, FCrossroad>	crossroads;
	TMap<FVector, int>		crossroadIds;

	int nextSegmentId = 0;
	int nextCrossroadId = 0;

	//base processing; search for common start/end points
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

	if (segments.Num() > 1)
	{
		//search for missing crossroads
		TMap<FVector, bool> pointsTypes; //bool - isCrossroad
		for (auto& segmentData : segments)
		{
			auto segmentPoints = segmentData.Value.AllPoints;
			for (auto& point : segmentPoints)
			{
				bool isAlreadyCrossroad = crossroadIds.Contains(point);
				bool* isAlreadyCrossroadRef = pointsTypes.Find(point);
				if (isAlreadyCrossroadRef == nullptr)
				{
					pointsTypes.Add(point, isAlreadyCrossroad);
				}
				else
				{
					if (!*isAlreadyCrossroadRef)
					{
						*isAlreadyCrossroadRef = true;
						crossroadIds.Add(point, nextCrossroadId);
						crossroads.Add(nextCrossroadId++, FCrossroad{ point });
					}
				}
			}
		}

		//splitting segments
		for (int segmentId = 0; segmentId < nextSegmentId; segmentId++)
		{
			auto segment = segments[segmentId];
			TArray<int> splitIndeces;
			TArray<int> splitCrossroadsIds;
			for (int i = 1; i < segment.AllPoints.Num() - 1; i++)
			{
				auto point = segment.AllPoints[i];
				auto crossroadIdRef = crossroadIds.Find(point);
				if (crossroadIdRef != nullptr)
				{
					splitIndeces.Add(i);
					splitCrossroadsIds.Add(*crossroadIdRef);
				}
			}

			//separate segments from end
			for (int i = splitIndeces.Num() - 1; i >= 0; i--)
			{
				auto splitIndex = splitIndeces[i];
				auto splitCrossroadId = splitCrossroadsIds[i];
				auto newSegment = segment;
				auto newSegmentId = nextSegmentId++;

				newSegment.AllPoints.RemoveAt(0, splitIndex);
				segment.AllPoints.RemoveAt(splitIndex + 1, segment.AllPoints.Num() - (splitIndex + 1));
				segment.EndCrossroadId = splitCrossroadId;
				newSegment.StartCrossroadId = splitCrossroadId;

				auto firstCrossroadId = segment.StartCrossroadId;
				auto secondCrossroadId = newSegment.EndCrossroadId;

				auto splitCrossroad		= *crossroads.Find(splitCrossroadId);
				auto firstCrossroad		= *crossroads.Find(firstCrossroadId);
				auto secondCrossroad	= *crossroads.Find(secondCrossroadId);

				splitCrossroad.Roads.Add(segmentId, firstCrossroadId);
				splitCrossroad.Roads.Add(newSegmentId, secondCrossroadId);

				firstCrossroad.Roads.Emplace(segmentId, splitCrossroadId);

				secondCrossroad.Roads.Remove(segmentId);
				secondCrossroad.Roads.Add(newSegmentId, splitCrossroadId);

				crossroads.Emplace(splitCrossroadId,	splitCrossroad);
				crossroads.Emplace(firstCrossroadId,	firstCrossroad);
				crossroads.Emplace(secondCrossroadId,	secondCrossroad);

				segments.Add(newSegmentId, newSegment);
			}

			if (splitIndeces.Num() > 0)
			{
				segments.Emplace(segmentId, segment);
			}
		}
	}

	return FRoadNetwork{ segments, crossroads };
}


TArray<FContour> ULoaderHelper::FixRelationContours(TArray<FContour>& inUnclosedContours)
{
	TArray<FContour> closedContours = {};
	if (inUnclosedContours.Num() > 0)
	{
		closedContours.Add(inUnclosedContours[0]);
		inUnclosedContours.RemoveAt(0);
	}
	while (inUnclosedContours.Num() > 0)
	{
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
