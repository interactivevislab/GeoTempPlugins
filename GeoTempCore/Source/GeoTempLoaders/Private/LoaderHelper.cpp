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
	//for (auto& contour : closedContours)
	//{
	//	if (!contour.IsClosed())
	//	{
	//		//contour.FixClockwise();

	//		//for (int i = 1; i < contour.Points.Num()-3; i++)
	//		//{

	//		//}
	//	}
	//}
	return closedContours;
}

bool IsPointInPolygon(TArray<FVector> simPoly, FVector point)
{
	int j = simPoly.Num() - 1;
	bool oddNodes = false;
	for (int i = 0; i < simPoly.Num(); i++)
	{
		if ((simPoly[i].Y < point.Y && simPoly[j].Y >= point.Y || simPoly[j].Y < point.Y && simPoly[i].Y >= point.Y) && (simPoly[i].X <= point.X || simPoly[j].X <= point.X))
		{
			oddNodes ^= (simPoly[i].X + (point.Y - simPoly[i].Y) / (simPoly[j].Y - simPoly[i].Y) * (simPoly[j].X - simPoly[i].X) < point.X);
		}
		j = i;
	}

	return oddNodes;
}

double SignedArea(const TArray<FVector> &p)
{
	double A = 0;
	//========================================================//
	// Assumes:                                               //
	//    N+1 vertices:   p[0], p[1], ... , p[N-1], p[N]      //
	//    Closed polygon: p[0] = p[N]                         //
	// Returns:                                               //
	//    Signed area: -ve if anticlockwise, +ve if clockwise //
	//========================================================//
	int N = p.Num() - 1;
	for (int i = 0; i < N; i++) A += p[i].X * p[i + 1].Y - p[i + 1].X * p[i].Y;
	A *= 0.5;
	return A;
}

bool SameSign(double x, double y)
{
	return (x >= 0) ^ (y < 0);
}

// Returns 1 if point is to the right, -1 if point is to the left, 0 if point is on the line
double RelativePosition(FVector lineStart, FVector lineEnd, FVector point)
{
	return FMath::Sign((lineEnd.X - lineStart.X) * (point.Y - lineStart.Y) - (lineEnd.Y - lineStart.Y) * (point.X - lineStart.X));
}

TArray<FContour> ULoaderHelper::FixAndCutRelationContours(TArray<FContour>& inUnclosedContours, FVector4 inBounds, OsmRelation& inRelation, bool& outGoodData, TSet<int>& outErrorRelations)
{
	auto closedContours = ULoaderHelper::FixRelationContours(inUnclosedContours);
	FVector topLeft = FVector(inBounds.X, inBounds.Z, 0);
	FVector bottomLeft = FVector(inBounds.X, inBounds.W, 0);
	FVector topRight = FVector(inBounds.Y, inBounds.Z, 0);
	FVector bottomRight = FVector(inBounds.Y, inBounds.W, 0);
	TArray<FVector> boundsPolygon = {
		topLeft,
		bottomLeft,
		bottomRight,
		topRight,
		topLeft
	};
	TArray<FVector> boundsCorners = {
		topLeft,
		topRight,
		bottomRight,
		bottomLeft,
	};

	bool recordingPoints = false;
	TArray<FContour> cutPolygons = {};
	TArray<double> cutPolysDirections = {};
	TArray<FContour> resultPolygons = {};
	TArray<FVector> cutPoints = {};
	for (auto& contour : closedContours)
	{
		if (!contour.IsClosed())
		{
			outGoodData = outGoodData && false;
			outErrorRelations.Add(inRelation.Id);
			continue;

			//cutPoints.Empty();
			//for (int i = 0; i < contour.Points.Num(); i++)
			//{
			//	if (IsPointInPolygon(boundsPolygon, contour.Points[i]))
			//	{
			//		if (!recordingPoints)
			//		{
			//			recordingPoints = true;
			//			if (i>0)
			//			{
			//				FVector lineStart	= FVector(0);
			//				FVector lineEnd		= FVector(0);
			//				if (contour.Points[i - 1].X < inBounds.X)
			//				{
			//					lineStart.X	= inBounds.X;
			//					//lineEnd.X	= inBounds.X;
			//				}
			//				else
			//				{
			//					lineStart.X	= inBounds.Y;
			//					//lineEnd.X	= inBounds.Y;
			//				}
			//				if (contour.Points[i - 1].Y < inBounds.Z)
			//				{
			//					lineStart.Y	= inBounds.Z;
			//					//lineEnd.Y	= inBounds.Z;
			//				}
			//				else
			//				{
			//					lineStart.Y	= inBounds.W;
			//					//lineEnd.Y	= inBounds.W;
			//				}
			//				auto position = RelativePosition(contour.Points[i - 1], lineStart, contour.Points[i]);
			//				if (position<=0)
			//				{
			//					lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.Y : inBounds.X;
			//					lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.Z : inBounds.W;
			//				}
			//				else
			//				{
			//					lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.X : inBounds.Y;
			//					lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.W : inBounds.Z;
			//				}
			//				FVector intersection;
			//				auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, contour.Points[i - 1], contour.Points[i], intersection);
			//				if (foundIntersection)
			//				{
			//					cutPoints.Add(intersection);
			//				}
			//				else
			//				{
			//					cutPoints.Add(contour.Points[i - 1]);
			//				}
			//			}
			//		}
			//		cutPoints.Add(contour.Points[i]);
			//	}
			//	else
			//	{
			//		if (recordingPoints)
			//		{
			//			recordingPoints = false;
			//			FVector lineStart = FVector(0);
			//			FVector lineEnd = FVector(0);
			//			if (contour.Points[i].X < inBounds.X)
			//			{
			//				lineStart.X = inBounds.X;
			//				//lineEnd.X	= inBounds.X;
			//			}
			//			else
			//			{
			//				lineStart.X = inBounds.Y;
			//				//lineEnd.X	= inBounds.Y;
			//			}
			//			if (contour.Points[i].Y < inBounds.Z)
			//			{
			//				lineStart.Y = inBounds.Z;
			//				//lineEnd.Y	= inBounds.Z;
			//			}
			//			else
			//			{
			//				lineStart.Y = inBounds.W;
			//				//lineEnd.Y	= inBounds.W;
			//			}
			//			auto position = RelativePosition(contour.Points[i], lineStart, contour.Points[i-1]);
			//			if (position <= 0)
			//			{
			//				lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.Y : inBounds.X;
			//				lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.Z : inBounds.W;
			//			}
			//			else
			//			{
			//				lineEnd.X = (lineStart.Y == inBounds.Z) ? inBounds.X : inBounds.Y;
			//				lineEnd.Y = (lineStart.X == inBounds.X) ? inBounds.W : inBounds.Z;
			//			}

			//			FVector intersection;
			//			auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, contour.Points[i - 1], contour.Points[i], intersection);
			//			if (foundIntersection)
			//			{
			//				cutPoints.Add(intersection);
			//			}
			//			else
			//			{
			//				cutPoints.Add(contour.Points[i]);
			//			}
			//			auto cont = FContour(cutPoints);
			//			//cont.FixClockwise();
			//			cutPolygons.Add(cont);
			//			cutPoints.Empty();
			//		}
			//	}
			//}
			//////auto cont = FContour(cutPoints);
			//////cont.FixClockwise();
			//////cutPolygons.Add(cont);
		}
		else
		{
			for (int i = 0; i < contour.Points.Num(); i++)
			{
				if (IsPointInPolygon(boundsPolygon, contour.Points[i]))
				{
					if (!recordingPoints)
					{
						recordingPoints = true;
						if (i > 0)
						{
							FVector lineStart = FVector(0);
							FVector lineEnd = FVector(0);
							if (contour.Points[i - 1].X < inBounds.X)
							{
								lineStart.X = inBounds.X;
								//lineEnd.X	= inBounds.X;
							}
							else
							{
								lineStart.X = inBounds.Y;
								//lineEnd.X	= inBounds.Y;
							}
							if (contour.Points[i - 1].Y < inBounds.Z)
							{
								lineStart.Y = inBounds.Z;
								//lineEnd.Y	= inBounds.Z;
							}
							else
							{
								lineStart.Y = inBounds.W;
								//lineEnd.Y	= inBounds.W;
							}
							auto position = RelativePosition(contour.Points[i - 1], lineStart, contour.Points[i]);
							if (position <= 0)
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
							auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, contour.Points[i - 1], contour.Points[i], intersection);
							if (foundIntersection)
							{
								cutPoints.Add(intersection);
							}
							else
							{
								cutPoints.Add(contour.Points[i - 1]);
							}
						}
					}
					cutPoints.Add(contour.Points[i]);
				}
				else
				{
					if (recordingPoints)
					{
						recordingPoints = false;
						FVector lineStart = FVector(0);
						FVector lineEnd = FVector(0);
						if (contour.Points[i].X < inBounds.X)
						{
							lineStart.X = inBounds.X;
							//lineEnd.X	= inBounds.X;
						}
						else
						{
							lineStart.X = inBounds.Y;
							//lineEnd.X	= inBounds.Y;
						}
						if (contour.Points[i].Y < inBounds.Z)
						{
							lineStart.Y = inBounds.Z;
							//lineEnd.Y	= inBounds.Z;
						}
						else
						{
							lineStart.Y = inBounds.W;
							//lineEnd.Y	= inBounds.W;
						}
						auto position = RelativePosition(contour.Points[i], lineStart, contour.Points[i - 1]);
						if (position <= 0)
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
						auto foundIntersection = UGeometryHelpers::DoLineSegmentsIntersect(lineStart, lineEnd, contour.Points[i - 1], contour.Points[i], intersection);
						if (foundIntersection)
						{
							cutPoints.Add(intersection);
						}
						else
						{
							cutPoints.Add(contour.Points[i]);
						}
						auto cont = FContour(cutPoints);
						//cont.FixClockwise();
						cutPolygons.Add(cont);
						cutPolysDirections.Add(SignedArea(contour.Points));
						cutPoints.Empty();
					}
				}
			}
			if (cutPoints.Num()>0)
			{
				if (cutPolygons.Num()>0)
				{
					cutPolygons[0].Points.RemoveAt(0);
					cutPolygons[0].Points.Insert(cutPoints, 0);
				}
				else
				{
					auto cont = FContour(cutPoints);
					cutPolygons.Add(cont);
					cutPolysDirections.Add(SignedArea(contour.Points));
				}
				cutPoints.Empty();
			}
			////auto cont = FContour(cutPoints);
			////cont.FixClockwise();
			////cutPolygons.Add(cont);




			//resultPolygons.Add(contour);
		}
	}

	TArray<FContour> combinedContours = {};
	TArray<double> combinedContsDirection = {};
	if (cutPolygons.Num() > 0)
	{
		combinedContours.Add(cutPolygons[0]);
		combinedContsDirection.Add(cutPolysDirections[0]);
		cutPolygons.RemoveAt(0);
		cutPolysDirections.RemoveAt(0);
	}
	while (cutPolygons.Num() > 0)
	{
		bool hasConnections = false;

		for (int j = 0; j<combinedContours.Num();j++)
		{
			auto lastPointIndex = combinedContours[j].Points.Num() - 1;
			auto lastPoint = combinedContours[j].Points[lastPointIndex];
			int contourToRemove = -1;
			for (int i = 0; i < cutPolygons.Num(); i++)
			{
				auto relativePosition = RelativePosition(combinedContours[j].Points[0], lastPoint, cutPolygons[i].Points[0]);
				if (relativePosition == 0)
				{
					combinedContours[j].Points.Append(cutPolygons[i].Points);
					contourToRemove = i;
					break;
				}
				else
				{
					if (SameSign(relativePosition, combinedContsDirection[j]))
					{
						TArray<FVector> corners = {};
						int startIndex = 0;
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
							if (combinedContsDirection[j] >= 0)
							{
								borderDirection = k;
							}
							else
							{
								borderDirection = boundsCorners.Num() - k;
							}
							int newIndex = (borderDirection + startIndex) % 4;
							auto cornerRelativePosition = RelativePosition(lastPoint, cutPolygons[i].Points[0], boundsCorners[newIndex]);
							auto cornerCheckSideCorrection = -FMath::Sign(combinedContsDirection[j]);

							if (cornerRelativePosition != 0 && SameSign(cornerCheckSideCorrection*cornerRelativePosition, combinedContsDirection[j]))
							{
								corners.Add(boundsCorners[newIndex]);
							}
						}
						combinedContours[j].Points.Append(corners);


						combinedContours[j].Points.Append(cutPolygons[i].Points);
						contourToRemove = i;
						break;
					}
				}
				
			}
			
			if (contourToRemove >= 0)
			{
				cutPolygons.RemoveAt(contourToRemove);
				hasConnections = true;
			}
			else
			{
				hasConnections = false;
			}
		}
		if (!hasConnections)
		{
			combinedContours.Add(cutPolygons[0]);
			combinedContsDirection.Add(cutPolysDirections[0]);
			cutPolygons.RemoveAt(0);
			cutPolysDirections.RemoveAt(0);
			continue;
		}
	}
	for (int j = 0; j < combinedContours.Num(); j++)
	{
		TArray<FVector> corners = {};
		int startIndex = 0;
		auto lastPoint = combinedContours[j].Points[combinedContours[j].Points.Num() - 1];
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
			if (combinedContsDirection[j] >= 0)
			{
				borderDirection = i;
			}
			else
			{
				borderDirection = boundsCorners.Num() - i;
			}
			int newIndex = (borderDirection + startIndex) % 4;
			auto cornerRelativePosition = RelativePosition(combinedContours[j].Points[0], lastPoint, boundsCorners[newIndex]);
			auto cornerCheckSideCorrection = -FMath::Sign(combinedContsDirection[j]);

			if (cornerRelativePosition!=0 && SameSign(cornerRelativePosition, combinedContsDirection[j]))
			{
				corners.Add(boundsCorners[newIndex]);
			}
		}



		/*bool gap = false;
		if (RelativePosition(contour.Points[0], contour.Points[contour.Points.Num() - 1], topLeft) >= 0)
		{
			corners.Add(topLeft);
		}
		if (contour.Points[0].Y == topLeft.Y)
		{
			gap = true;
		}
		if (RelativePosition(contour.Points[0], contour.Points[contour.Points.Num() - 1], topRight) >= 0)
		{
			corners.Insert(topRight, gap ? 0 : corners.Num());
		}
		else
		{
			if (corners.Num()>0)
			{
				gap = true;
			}
		}
		if (contour.Points[0].X == topRight.X)
		{
			gap = true;
		}
		if (RelativePosition(contour.Points[0], contour.Points[contour.Points.Num() - 1], bottomRight) >= 0)
		{
			corners.Insert(bottomRight, gap ? 0 : corners.Num());
		}
		else
		{
			if (corners.Num() > 0)
			{
				gap = true;
			}
		}
		if (contour.Points[0].Y == bottomRight.Y)
		{
			gap = true;
		}
		if (RelativePosition(contour.Points[0], contour.Points[contour.Points.Num() - 1], bottomLeft) >= 0)
		{
			corners.Insert(bottomLeft, gap ? 0 : corners.Num());
		}*/

		combinedContours[j].Points.Append(corners);
		combinedContours[j].FixClockwise();
		resultPolygons.Add(combinedContours[j]);
	}
	//if (cutPoints.Num()>0)
	//{
	//	auto cont = FContour(cutPoints);
	//	cont.FixClockwise();
	//	resultPolygons.Add(cont);
	//}
	return resultPolygons;
}
