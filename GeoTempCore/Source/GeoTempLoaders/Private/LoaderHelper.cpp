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


TArray<FContour> ULoaderHelper::FixRelationContours(TArray<FContour>& inUnclosedContours, int inRelationId, bool& outGoodData, TSet<int>& outErrorRelations)
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
			outGoodData = outGoodData && false;
			outErrorRelations.Add(inRelationId);
			continue;
		}
	}
	return closedContours;
}


TArray<FContour> ULoaderHelper::FixAndCutRelationContours(TArray<FContour>& inUnclosedContours, FVector4 inBounds, int inRelationId, bool& outGoodData, TSet<int>& outErrorRelations)
{
	auto closedContours = ULoaderHelper::FixRelationContours(inUnclosedContours, inRelationId, outGoodData, outErrorRelations);
	return ULoaderHelper::CutPolygonsByBounds(closedContours, inBounds);
}


TArray<FContour> ULoaderHelper::CutPolygonsByBounds(TArray<FContour>& inContours, FVector4 inBounds)
{
	TArray<FContour> cutPolygons = {};
	for (auto polygon : inContours)
	{
		auto cutPolygon = ULoaderHelper::CutPolygonByBounds(polygon, inBounds);
		for (auto polygonParts : cutPolygon)
		{
			cutPolygons.Add(polygonParts);
		}
	}
	return cutPolygons;
}


FVector ULoaderHelper::GetSegmentIntersectionWithBounds(FVector inOuterPoint, FVector inInnerPoint, FVector4 inBounds)
{
	FVector lineStart	= FVector(0);
	FVector lineEnd		= FVector(0);

	if (inOuterPoint.X < inBounds.X)
	{
		lineStart.X = inBounds.X;
	}
	else
	{
		lineStart.X = inBounds.Y;
	}
	if (inOuterPoint.Y < inBounds.Z)
	{
		lineStart.Y = inBounds.Z;
	}
	else
	{
		lineStart.Y = inBounds.W;
	}

	auto pointRelativePosition = UGeometryHelpers::PointToLineRelativePosition(inOuterPoint, lineStart, inInnerPoint);
	if (pointRelativePosition <= 0)
	{
		lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.Y : inBounds.X;
		lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.Z : inBounds.W;
	}
	else
	{
		lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.X : inBounds.Y;
		lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.W : inBounds.Z;
	}

	FVector intersection;
	auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, inOuterPoint, inInnerPoint, intersection);
	if (foundIntersection)
	{
		return intersection;
	}
	else
	{
		return inOuterPoint;
	}
}


TArray<FContour> ULoaderHelper::CutContourByBounds(FContour inContour, FVector4 inBounds, TArray<FVector>& outIntersectionPoints)
{
	FVector topLeftBoundsCorner		= FVector(inBounds.X, inBounds.Z, 0);
	FVector bottomLeftBoundsCorner	= FVector(inBounds.X, inBounds.W, 0);
	FVector topRightBoundsCorner	= FVector(inBounds.Y, inBounds.Z, 0);
	FVector bottomRightBoundsCorner	= FVector(inBounds.Y, inBounds.W, 0);

	TArray<FVector> cutPoints		= {};
	TArray<FContour> cutContours	= {};

	TArray<FVector> boundsPolygon	= {
	topLeftBoundsCorner,
	bottomLeftBoundsCorner,
	bottomRightBoundsCorner,
	topRightBoundsCorner,
	topLeftBoundsCorner
	};

	bool recordingPoints		= false;
	bool allPointsInsideBounds	= true;

	for (int i = 0; i < inContour.Points.Num(); i++)
	{
		if (UGeometryHelpers::IsPointInPolygon(boundsPolygon, inContour.Points[i]))
		{
			if (!recordingPoints)
			{
				recordingPoints = true;
				if (i > 0)
				{
					/*FVector lineStart = FVector(0);
					FVector lineEnd = FVector(0);
					if (inContour.Points[i - 1].X < inBounds.X)
					{
						lineStart.X = inBounds.X;
					}
					else
					{
						lineStart.X = inBounds.Y;
					}
					if (inContour.Points[i - 1].Y < inBounds.Z)
					{
						lineStart.Y = inBounds.Z;
					}
					else
					{
						lineStart.Y = inBounds.W;
					}
					auto pointRelativePosition = UGeometryHelpers::PointToLineRelativePosition(inContour.Points[i - 1], lineStart, inContour.Points[i]);
					if (pointRelativePosition <= 0)
					{
						lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.Y : inBounds.X;
						lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.Z : inBounds.W;
					}
					else
					{
						lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.X : inBounds.Y;
						lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.W : inBounds.Z;
					}
					FVector intersection;
					auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, inContour.Points[i - 1], inContour.Points[i], intersection);
					if (foundIntersection)
					{
						cutPoints.Add(intersection);
					}
					else
					{
						cutPoints.Add(inContour.Points[i - 1]);
					}*/
					auto intersection = ULoaderHelper::GetSegmentIntersectionWithBounds(inContour.Points[i - 1], inContour.Points[i], inBounds);
					cutPoints.Add(intersection);
					outIntersectionPoints.Add(intersection);
				}
			}
			cutPoints.Add(inContour.Points[i]);
		}
		else
		{
			allPointsInsideBounds = false;
			if (recordingPoints)
			{
				recordingPoints = false;

				/*FVector lineStart = FVector(0);
				FVector lineEnd = FVector(0);
				if (inContour.Points[i].X < inBounds.X)
				{
					lineStart.X = inBounds.X;
				}
				else
				{
					lineStart.X = inBounds.Y;
				}
				if (inContour.Points[i].Y < inBounds.Z)
				{
					lineStart.Y = inBounds.Z;
				}
				else
				{
					lineStart.Y = inBounds.W;
				}
				auto pointRelativePosition = UGeometryHelpers::PointToLineRelativePosition(inContour.Points[i], lineStart, inContour.Points[i - 1]);
				if (pointRelativePosition <= 0)
				{
					lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.Y : inBounds.X;
					lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.Z : inBounds.W;
				}
				else
				{
					lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.X : inBounds.Y;
					lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.W : inBounds.Z;
				}

				FVector intersection;
				auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, inContour.Points[i - 1], inContour.Points[i], intersection);
				if (foundIntersection)
				{
					cutPoints.Add(intersection);
				}
				else
				{
					cutPoints.Add(inContour.Points[i]);
				}*/
				auto intersection = ULoaderHelper::GetSegmentIntersectionWithBounds(inContour.Points[i], inContour.Points[i - 1], inBounds);
				cutPoints.Add(intersection);
				outIntersectionPoints.Add(intersection);

				cutContours.Add(FContour(cutPoints));
				cutPoints.Empty();
			}
		}
	}
	if (cutPoints.Num() > 0)
	{
		if (allPointsInsideBounds || cutContours.Num() == 0)
		{
			//auto untouchedContour = FContour(cutPoints);
			cutContours.Add(FContour(cutPoints));
		}
		else
		{
			//if (cutContours.Num() > 0)
			//{
				cutContours[0].Points.RemoveAt(0);
				cutContours[0].Points.Insert(cutPoints, 0);
			//}
			//else
			//{
			//	cutContours.Add(FContour(cutPoints));
			//}
		}
		cutPoints.Empty();
	}
	return cutContours;
}

TArray<FContour> ULoaderHelper::CutPolygonByBounds(FContour inContour, FVector4 inBounds)
{
	FVector topLeftBoundsCorner		= FVector(inBounds.X, inBounds.Z, 0);
	FVector bottomLeftBoundsCorner	= FVector(inBounds.X, inBounds.W, 0);
	FVector topRightBoundsCorner	= FVector(inBounds.Y, inBounds.Z, 0);
	FVector bottomRightBoundsCorner	= FVector(inBounds.Y, inBounds.W, 0);


	TArray<FVector> boundsPolygon	= {
		topLeftBoundsCorner,
		bottomLeftBoundsCorner,
		bottomRightBoundsCorner,
		topRightBoundsCorner,
		topLeftBoundsCorner
	};
	TArray<FVector> boundsCorners	= {
		topLeftBoundsCorner,
		topRightBoundsCorner,
		bottomRightBoundsCorner,
		bottomLeftBoundsCorner,
	};

	TArray<FContour> cutContours		= {};
	TArray<FContour> combinedContours	= {};
	TArray<FContour> resultPolygons		= {};

	double polygonDirection;

	if (inContour.IsClosed())
	{
		TArray<FVector> intersections;
		cutContours			= ULoaderHelper::CutContourByBounds(inContour, inBounds, intersections);
		polygonDirection	= UGeometryHelpers::PolygonDirectionSign(inContour.Points);
	}
	
	if (cutContours.Num() > 0)
	{
		combinedContours.Add(cutContours[0]);
		cutContours.RemoveAt(0);
	}
	while (cutContours.Num() > 0)
	{
		bool hasConnections = false;

		for (int j = 0; j < combinedContours.Num(); j++)
		{
			auto lastPointIndex			= combinedContours[j].Points.Num() - 1;
			auto lastPoint				= combinedContours[j].Points[lastPointIndex];
			int contourToRemoveIndex	= -1;

			for (int i = 0; i < cutContours.Num(); i++)
			{
				auto nextContourRelativePosition = UGeometryHelpers::PointToLineRelativePosition(combinedContours[j].Points[0], lastPoint, cutContours[i].Points[0]);
				if (nextContourRelativePosition == 0)
				{
					auto startNewDiff = (combinedContours[j].Points[0] - cutContours[i].Points[0]);
					auto startEndDiff = (combinedContours[j].Points[0] - lastPoint);

					if	(	UGeometryHelpers::HasNumbersSameSign(startNewDiff.X, startEndDiff.X) 
						&&	UGeometryHelpers::HasNumbersSameSign(startNewDiff.Y, startEndDiff.Y)
						)
					{
						combinedContours[j].Points.Append(cutContours[i].Points);
						contourToRemoveIndex = i;
						break;
					}
				}
				else
				{
					if (UGeometryHelpers::HasNumbersSameSign(nextContourRelativePosition, polygonDirection))
					{
						TArray<FVector> corners	= {};
						int startIndex			= 0;

						if (lastPoint.Y == inBounds.Z)
						{
							startIndex += 0;
						}
						else
						{
							startIndex += 2;
						}
						if (lastPoint.X == inBounds.X)
						{
							startIndex += (startIndex == 2) ? 1 : 0;
						}
						else
						{
							startIndex += (startIndex == 2) ? 0 : 1;
						}

						for (int k = 0; k < boundsCorners.Num(); k++)
						{
							int borderDirection;
							if (polygonDirection >= 0)
							{
								borderDirection = k;
							}
							else
							{
								borderDirection = boundsCorners.Num() - k;
							}
							int newIndex				= (borderDirection + startIndex) % 4;
							auto cornerRelativePosition	= UGeometryHelpers::PointToLineRelativePosition(lastPoint, cutContours[i].Points[0], boundsCorners[newIndex]);

							if (cornerRelativePosition != 0 && UGeometryHelpers::HasNumbersSameSign(cornerRelativePosition, -polygonDirection))
							{
								corners.Add(boundsCorners[newIndex]);
							}
						}
						combinedContours[j].Points.Append(corners);
						combinedContours[j].Points.Append(cutContours[i].Points);
						contourToRemoveIndex = i;
						break;
					}
				}
			}

			if (contourToRemoveIndex >= 0)
			{
				cutContours.RemoveAt(contourToRemoveIndex);
				hasConnections = true;
			}
			else
			{
				hasConnections = false;
			}
		}

		if (!hasConnections)
		{
			combinedContours.Add(cutContours[0]);
			cutContours.RemoveAt(0);
			continue;
		}
	}

	for (int j = 0; j < combinedContours.Num(); j++)
	{
		TArray<FVector> corners	= {};
		int startIndex			= 0;
		auto lastPoint			= combinedContours[j].Points[combinedContours[j].Points.Num() - 1];

		if (lastPoint.Y == inBounds.Z)
		{
			startIndex += 0;
		}
		else
		{
			startIndex += 2;
		}
		if (lastPoint.X == inBounds.X)
		{
			startIndex += (startIndex == 2) ? 1 : 0;
		}
		else
		{
			startIndex += (startIndex == 2) ? 0 : 1;
		}

		for (int i = 0; i < boundsCorners.Num(); i++)
		{
			int borderDirection;
			if (polygonDirection >= 0)
			{
				borderDirection = i;
			}
			else
			{
				borderDirection = boundsCorners.Num() - i;
			}
			int newIndex				= (borderDirection + startIndex) % 4;
			auto cornerRelativePosition	= UGeometryHelpers::PointToLineRelativePosition(lastPoint, combinedContours[j].Points[0], boundsCorners[newIndex]);

			if (cornerRelativePosition != 0 && UGeometryHelpers::HasNumbersSameSign(cornerRelativePosition, -polygonDirection))
			{
				corners.Add(boundsCorners[newIndex]);
			}
		}
		combinedContours[j].Points.Append(corners);
		combinedContours[j].FixClockwise();
		resultPolygons.Add(combinedContours[j]);
	}
	return resultPolygons;
}
