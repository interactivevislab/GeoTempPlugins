#include "RoadBuilder.h"

#include "RoadNetworkActor.h"

#include "Materials/MaterialInstanceDynamic.h"


#define LIST_2_TIMES(something) something, something
#define LIST_4_TIMES(something) LIST_2_TIMES(something), LIST_2_TIMES(something)
#define LIST_8_TIMES(something) LIST_4_TIMES(something), LIST_4_TIMES(something)


const float URoadBuilder::ArcsAngleStep = PI / 18;
const int URoadBuilder::DefaultCapDensity = 8;


struct RoadSegmentGeometry
{
	int StartCrossroadId;
	int EndCrossroadId;
	int Lanes;
	float Width;
	float TurningRadius;
	float DistanceFromStartCrossroad = 0;
	float DistanceFromEndCrossroad = 0;
	bool IsValid = true;
};


enum class ArcType
{
	INNER_ARC,
	OUTER_ARC,
	NO_ARC
};


struct CrossroadGeometry
{
	FVector Center;
	TArray<int> SegmentsIds;
	TArray<ArcType> Arcs;
	TArray<FVector> ArcsCenters;
};


struct RoadNetworkGeometry
{
	TMap<int, CrossroadGeometry> Crossroads;
	TMap<int, RoadSegmentGeometry> Segments;
};


RoadSegmentGeometry GetSegmentGeometry(FRoadSegment inSegment)
{
	float speedOnSegment = 8.0f; // m/s
	float turningRadius = 18.6f * FMath::Sqrt(speedOnSegment / FMath::Abs(10 * inSegment.Width + 65 - speedOnSegment));
	return { 0, 0, inSegment.Lanes, 100 * inSegment.Width, 100 * turningRadius / 4 };
}


RoadNetworkGeometry GetRoadNetworkGeometry(FRoadNetwork inRoadNetwork)
{
	RoadNetworkGeometry convertedGeometry;

	int lastCrossroadId = -1;
	int nextSegmentId = 0;
	
	//getting base crossroads
	for (auto crossroadData : inRoadNetwork.Crossroads)
	{
		auto id = crossroadData.Key;
		auto crossroad = crossroadData.Value;
		lastCrossroadId = FMath::Max(lastCrossroadId, id);
		convertedGeometry.Crossroads.Add(id, CrossroadGeometry{ crossroad.Location });
	}

	//cutting segments into minimal parts
	for (auto segmentData : inRoadNetwork.Segments)
	{
		auto segment = segmentData.Value;

		if (segment.Type != EHighwayType::Auto)
		{
			continue;
		}

		RoadSegmentGeometry currentSegmentGeometry = GetSegmentGeometry(segment);

		auto totalPoints = segment.AllPoints.Num();
		for (int i = 0; i < totalPoints - 1; i++)
		{
			if (i == 0)
			{
				auto id = segment.StartCrossroadId;
				auto crossroadBuffer = convertedGeometry.Crossroads[id];
				crossroadBuffer.SegmentsIds.Add(nextSegmentId);
				convertedGeometry.Crossroads.Emplace(id, crossroadBuffer);

				currentSegmentGeometry.StartCrossroadId = id;
			}
			else
			{
				currentSegmentGeometry.StartCrossroadId = lastCrossroadId;
			}

			if (i == totalPoints - 2)
			{
				auto id = segment.EndCrossroadId;
				auto crossroadBuffer = convertedGeometry.Crossroads[id];
				crossroadBuffer.SegmentsIds.Add(nextSegmentId);
				convertedGeometry.Crossroads.Emplace(id, crossroadBuffer);

				currentSegmentGeometry.EndCrossroadId = id;
			}
			else
			{
				currentSegmentGeometry.EndCrossroadId = ++lastCrossroadId;
				convertedGeometry.Crossroads.Add(lastCrossroadId,
					CrossroadGeometry { 
						segment.AllPoints[i + 1], 
						TArray<int>{ nextSegmentId, nextSegmentId + 1}
					});
			}

			convertedGeometry.Segments.Add(nextSegmentId++, currentSegmentGeometry);
		}
	}

	//segments sorting by angle
	for (auto crossroadData : convertedGeometry.Crossroads)
	{
		auto crossroadId = crossroadData.Key;
		auto crossroad = crossroadData.Value;

		TMap<float, int> atansMap;
		TArray<float> atans;
		for (auto segmentsId : crossroad.SegmentsIds)
		{
			auto segment = convertedGeometry.Segments[segmentsId];
			auto otherCrossroadId = (segment.StartCrossroadId == crossroadId)
				? segment.EndCrossroadId
				: segment.StartCrossroadId;
			auto segmentAsVector = convertedGeometry.Crossroads[otherCrossroadId].Center - crossroad.Center;
			auto atan = FMath::Atan2(segmentAsVector.Y, segmentAsVector.X);
			atans.Add(atan);
			atansMap.Add(atan, segmentsId);
		}

		atans.Sort();

		CrossroadGeometry sortedCrossroadGeometry { crossroad.Center };
		for (auto atan : atans)
		{
			sortedCrossroadGeometry.SegmentsIds.Add(*atansMap.Find(atan));
		}

		convertedGeometry.Crossroads.Emplace(crossroadId, sortedCrossroadGeometry);
	}

	//calculation crossroads borders
	for (auto crossroadData : convertedGeometry.Crossroads)
	{
		auto crossroadId = crossroadData.Key;
		auto crossroad = crossroadData.Value;
		auto centerPoint = crossroad.Center;
		auto segmentsNum = crossroad.SegmentsIds.Num();

		if (segmentsNum < 2)
		{
			continue;
		}

		for (int i = 0; i < segmentsNum; i++)
		{
			auto firstSegmentId = crossroad.SegmentsIds[i];
			auto secondSegmentId = crossroad.SegmentsIds[(i + 1) % segmentsNum];
			auto firstSegment = convertedGeometry.Segments[firstSegmentId];
			auto secondSegment = convertedGeometry.Segments[secondSegmentId];

			bool isStartForFirst = (firstSegment.StartCrossroadId == crossroadId);
			bool isStartForSecond = (secondSegment.StartCrossroadId == crossroadId);
			auto firstOtherCrossroadId = isStartForFirst ? firstSegment.EndCrossroadId : firstSegment.StartCrossroadId;
			auto secondOtherCrossroadId = isStartForSecond ? secondSegment.EndCrossroadId : secondSegment.StartCrossroadId;
			auto firstPoint = convertedGeometry.Crossroads[firstOtherCrossroadId].Center;
			auto secondPoint = convertedGeometry.Crossroads[secondOtherCrossroadId].Center;
			
			auto firstDirection = (firstPoint - centerPoint).GetSafeNormal();
			auto secondDirection = (secondPoint - centerPoint).GetSafeNormal();
			auto firstDelta = firstSegment.Width / 2 * FVector::CrossProduct(FVector::UpVector, firstDirection);
			auto secondDelta = secondSegment.Width / 2 * FVector::CrossProduct(FVector::DownVector, secondDirection);

			FVector pointBuffer;
			bool isSpecialCase = !FMath::SegmentIntersection2D(firstPoint + firstDelta, centerPoint + firstDelta,
				secondPoint + secondDelta, centerPoint + secondDelta, pointBuffer);

			auto dot = firstDirection.X * secondDirection.X + firstDirection.Y * secondDirection.Y;
			auto det = firstDirection.X * secondDirection.Y - firstDirection.Y * secondDirection.X;
			auto angle = FMath::Atan2(det, dot);	//in radians
			if (angle < 0)
			{
				angle += 2 * PI;
			}

			auto& firstDistance = isStartForFirst
				? firstSegment.DistanceFromStartCrossroad
				: firstSegment.DistanceFromEndCrossroad;
			auto& secondDistance = isStartForSecond
				? secondSegment.DistanceFromStartCrossroad
				: secondSegment.DistanceFromEndCrossroad;

			if (angle >= PI)
			{
				isSpecialCase = true;
				crossroad.Arcs.Add(ArcType::OUTER_ARC);
				crossroad.ArcsCenters.Add(FVector());
			}
			else if(isSpecialCase)
			{
				crossroad.Arcs.Add(ArcType::NO_ARC);
				crossroad.ArcsCenters.Add(FVector());
			}

			if (isSpecialCase)
			{		
				//negative values are "flags" for special cases
				auto specialDistance = -0.67f * FMath::Max(firstSegment.Width, secondSegment.Width);
				if (firstDistance <= 0)
				{
					firstDistance = FMath::Min(firstDistance, specialDistance);
					convertedGeometry.Segments.Emplace(firstSegmentId, firstSegment);
				}
				if (secondDistance <= 0)
				{
					secondDistance = FMath::Min(secondDistance, specialDistance);
					convertedGeometry.Segments.Emplace(secondSegmentId, secondSegment);
				}
			}
			else
			{
				auto turningRadius = FMath::Min(firstSegment.TurningRadius, secondSegment.TurningRadius);
				auto d1 = firstSegment.Width / 2 + turningRadius;
				auto d2 = secondSegment.Width / 2 + turningRadius;
				auto sinA = FMath::Sin(angle);
				auto P = FMath::Sqrt(d1 * d1 + d2 * d2 + 2 * d1 * d2 * FMath::Cos(angle));
				auto d1Angle = FMath::Asin(d1 / P * sinA);
				auto D1Angle = PI / 2 - d1Angle;
				auto D2Angle = PI - angle - D1Angle;
				auto D1 = P * FMath::Sin(D1Angle) / sinA;
				auto D2 = P * FMath::Sin(D2Angle) / sinA;

				if (firstDistance < D1)
				{
					firstDistance = D1;
					convertedGeometry.Segments.Emplace(firstSegmentId, firstSegment);
				}
				if (secondDistance < D2)
				{
					secondDistance = D2;
					convertedGeometry.Segments.Emplace(secondSegmentId, secondSegment);
				}

				auto arcCenter = centerPoint + D1 * firstDirection 
					+ d1 * FVector::CrossProduct(FVector::UpVector, firstDirection);
				crossroad.Arcs.Add(ArcType::INNER_ARC);
				crossroad.ArcsCenters.Add(arcCenter);
			}
		}
		
		convertedGeometry.Crossroads.Emplace(crossroadId, crossroad);
	}

	//fixing negative distance values
	for (auto segmentData : convertedGeometry.Segments)
	{
		auto segmentId = segmentData.Key;
		auto segment = segmentData.Value;

		bool needToEmplace = false;
		if (segment.DistanceFromStartCrossroad < 0)
		{
			segment.DistanceFromStartCrossroad *= -1;
			needToEmplace = true;
		}
		if (segment.DistanceFromEndCrossroad < 0)
		{
			segment.DistanceFromEndCrossroad *= -1;
			needToEmplace = true;
		}

		auto length = (convertedGeometry.Crossroads[segment.StartCrossroadId].Center 
			- convertedGeometry.Crossroads[segment.EndCrossroadId].Center).Size();
		if (segment.DistanceFromStartCrossroad + segment.DistanceFromEndCrossroad > length)
		{
			segment.IsValid = false;
			needToEmplace = true;
		}

		if (needToEmplace)
		{
			convertedGeometry.Segments.Emplace(segmentId, segment);
		}
	}

	return convertedGeometry;
}


struct MeshSectionData
{
	TArray<FVector>				Vertices;
	TArray<int>					Indices;
	TArray<FVector>				Normals;
	TArray<FVector2D>			Uv0;
	TArray<FVector2D>			Uv1;
	TArray<FColor>				VertexColors;
	TArray<FRuntimeMeshTangent>	Tangents;
};


void URoadBuilder::RemoveRoadNetworkActor()
{
	if (roadNetworkActor)
	{
		roadNetworkActor->Destroy();
		roadNetworkActor = nullptr;
	}

	TArray<ARoadNetworkActor*> toDestroy;
	for (auto child : GetOwner()->Children)
	{
		auto castChild = Cast<ARoadNetworkActor>(child);
		if (castChild)
		{
			toDestroy.Add(castChild);
		}
	}
	for (auto child : toDestroy)
	{
		child->Destroy();
	}
}


FVector URoadBuilder::CalculatePerpendicularToLine(FVector inStartPoint, FVector inEndPoint)
{
	return FVector::CrossProduct((inStartPoint - inEndPoint).GetSafeNormal(), FVector::UpVector);
}


TArray<FVector> URoadBuilder::ConvertLineToRect(FVector inStartPoint, FVector inEndPoint, FVector inPerpendicularToLine)
{
	return {
		inStartPoint	+ inPerpendicularToLine,
		inStartPoint	- inPerpendicularToLine,
		inEndPoint		+ inPerpendicularToLine,
		inEndPoint		- inPerpendicularToLine
	};
}


TArray<FVector2D> URoadBuilder::GetRoadCupsPointsDirections(int inCapDensity)
{
	TArray<FVector2D> pointsDirections = {};
	for (int j = 0; j <= inCapDensity; j++)
	{
		float angle = PI / inCapDensity * j;

		float x = FMath::Cos(angle);
		float y = FMath::Sin(angle);

		pointsDirections.Add(FVector2D(x, y));
	}
	return pointsDirections;
}


TArray<FVector> URoadBuilder::GetCupsPointsOffsets(TArray<FVector2D> inPointsDirections, FVector inPerpendicularToLine, bool inIsReversedCup)
{
	TArray<FVector> radiusDeltas = {};
	auto yDelta = FVector::CrossProduct(inPerpendicularToLine, FVector::UpVector);
	for (auto& pointDirection: inPointsDirections)
	{
		radiusDeltas.Add((inIsReversedCup ? 1 : -1) * (inPerpendicularToLine * pointDirection.X + yDelta * pointDirection.Y));
	}
	return radiusDeltas;
}


void FindSegmentToCrossroadConnection(RoadNetworkGeometry inNetworkGeometry, int inCrossroadId, int inSegmentId,
	bool isClockWise, FVector & outConnectionPoint, FVector & outBorderPoint, FVector & outDirectionPoint)
{
	auto crossroad	= inNetworkGeometry.Crossroads[inCrossroadId];
	auto segment	= inNetworkGeometry.Segments[inSegmentId];

	int otherCrossroadId;
	float distanceFromCenter;
	if (inCrossroadId == segment.StartCrossroadId)
	{
		otherCrossroadId = segment.EndCrossroadId;
		distanceFromCenter = segment.DistanceFromStartCrossroad;
	}
	else
	{
		otherCrossroadId = segment.StartCrossroadId;
		distanceFromCenter = segment.DistanceFromEndCrossroad;
	}

	auto otherCrossroadCenter = inNetworkGeometry.Crossroads[otherCrossroadId].Center;
	auto direction = (otherCrossroadCenter - crossroad.Center).GetSafeNormal();
	outConnectionPoint = crossroad.Center + distanceFromCenter * direction;
	auto delta = (isClockWise ? 1 : -1) * segment.Width / 2 * FVector::CrossProduct(FVector::UpVector, direction);
	outBorderPoint = outConnectionPoint + delta;
	outDirectionPoint = outBorderPoint + direction;
}


float GetDirectionAngleForOuterArc(FVector inCrossroadCenter, FVector inConnectionPoint, FVector inBorderPoint, 
	float inArcRadius)
{
	float a = (inCrossroadCenter - inConnectionPoint).Size();
	float b = (inConnectionPoint - inBorderPoint).Size();
	float pSquare = a * a + b * b;
	float c = FMath::Sqrt(pSquare - inArcRadius * inArcRadius);
	float d = inArcRadius;
	return FMath::Acos((a * a + d * d - b * b - c * c) / (a * d + b * c) / 2);
}


TPair<MeshSectionData, MeshSectionData> CalculateMeshDataForRoad(RoadNetworkGeometry inNetworkGeometry, 
	MeshSectionData& outCurtainsMeshData, float inRoadHeight, float inCurtainsWidth, float inStretch)
{
	MeshSectionData segmentsSectionData;
	MeshSectionData crossroadsSectionData;

	//mesh data for segments
	for (auto segmentData : inNetworkGeometry.Segments)
	{
		auto segmentId	= segmentData.Key;
		auto segment	= segmentData.Value;

		if (!segment.IsValid)
		{
			continue;
		}

		auto startPoint	= inNetworkGeometry.Crossroads[segment.StartCrossroadId].Center;
		auto endPoint	= inNetworkGeometry.Crossroads[segment.EndCrossroadId].Center;

		auto direction = (endPoint - startPoint).GetSafeNormal();
		startPoint += segment.DistanceFromStartCrossroad * direction;
		endPoint -= segment.DistanceFromEndCrossroad * direction;

		auto pointDelta = URoadBuilder::CalculatePerpendicularToLine(startPoint, endPoint);
		auto curtainsDelta1 =	pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
		auto curtainsDelta2 = -	pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);

		pointDelta *= segment.Width / 2;

		auto roadRectangle = URoadBuilder::ConvertLineToRect(startPoint, endPoint, pointDelta);

		auto indicesDelta = segmentsSectionData.Vertices.Num();
		segmentsSectionData.Vertices.Append(roadRectangle);

		segmentsSectionData.Indices.Append({
			0 + indicesDelta,
			2 + indicesDelta,
			1 + indicesDelta,
			3 + indicesDelta,
			1 + indicesDelta,
			2 + indicesDelta,
			});

		indicesDelta = outCurtainsMeshData.Vertices.Num();
		outCurtainsMeshData.Vertices.Append({
			roadRectangle[0] + curtainsDelta1,
			roadRectangle[0],
			roadRectangle[2] + curtainsDelta1,
			roadRectangle[2],

			roadRectangle[1],
			roadRectangle[1] + curtainsDelta2,
			roadRectangle[3],
			roadRectangle[3] + curtainsDelta2
			});

		outCurtainsMeshData.Indices.Append({
			0 + indicesDelta,
			2 + indicesDelta,
			1 + indicesDelta,
			3 + indicesDelta,
			1 + indicesDelta,
			2 + indicesDelta,
			});
		indicesDelta += 4;
		outCurtainsMeshData.Indices.Append({
			0 + indicesDelta,
			2 + indicesDelta,
			1 + indicesDelta,
			3 + indicesDelta,
			1 + indicesDelta,
			2 + indicesDelta,
			});

		//lenght in "width" units, rounded for not to cut texture
		auto lenght = FMath::Max(FMath::RoundToInt((startPoint - endPoint).Size()
			/ (segment.Width * inStretch) * segment.Lanes), 1);

		auto uv00 = FVector2D(0, 0);
		auto uv01 = FVector2D(segment.Lanes, 0);
		auto uv01c = FVector2D(1, 0);
		auto uv02 = FVector2D(0, lenght);
		auto uv03 = FVector2D(segment.Lanes, lenght);
		auto uv03c = FVector2D(1, lenght);

		segmentsSectionData.Uv0				.Append({ uv00, uv01, uv02, uv03 });
		segmentsSectionData.Uv1				.Append({ LIST_4_TIMES(FVector2D(0, 0)) });
		segmentsSectionData.Normals			.Append({ LIST_4_TIMES(FVector::UpVector) });
		segmentsSectionData.VertexColors	.Append({ LIST_4_TIMES(FColor(1, 1, 1, 1)) });
		segmentsSectionData.Tangents		.Append({ LIST_4_TIMES(FRuntimeMeshTangent()) });

		outCurtainsMeshData.Uv0				.Append({ uv00, uv01c, uv02, uv03c, uv00, uv01c, uv02, uv03c });
		outCurtainsMeshData.Uv1				.Append({ LIST_8_TIMES(FVector2D(0, 0)) });
		//normals are always Up for a good combination with the landscape
		outCurtainsMeshData.Normals			.Append({ LIST_8_TIMES(FVector::UpVector) });
		outCurtainsMeshData.VertexColors	.Append({ LIST_8_TIMES(FColor(1, 1, 1, 1)) });
		outCurtainsMeshData.Tangents		.Append({ LIST_8_TIMES(FRuntimeMeshTangent()) });
	}

	//mesh data for not segments (crossroads, turns, dead ends)
	for (auto crossroadsData : inNetworkGeometry.Crossroads)
	{
		auto crossroadId	= crossroadsData.Key;
		auto crossroad		= crossroadsData.Value;

		auto crossroadCenter = crossroad.Center;
		auto segmentsNum = crossroad.SegmentsIds.Num();

		//road turn or crossroad
		if (segmentsNum > 1)
		{
			TArray<FVector> mainPoints;
			TArray<FVector> curtainsPoints;
			TArray<int> curtainsSeparatorsIndeces;
			curtainsSeparatorsIndeces.Add(0);

			// variables for turn geometry data, recorded once
			int turnSeparatorPointIndex;
			RoadSegmentGeometry firstTurnSegment;
			RoadSegmentGeometry secondTurnSegment;

			for (int i = 0; i < segmentsNum; i++)
			{
				auto firstSegmentId		= crossroad.SegmentsIds[i];
				auto secondSegmentId	= crossroad.SegmentsIds[(i + 1) % segmentsNum];
				auto firstSegment		= inNetworkGeometry.Segments[firstSegmentId];
				auto secondSegment		= inNetworkGeometry.Segments[secondSegmentId];

				if (i == 1)
				{
					turnSeparatorPointIndex = mainPoints.Num();
					firstTurnSegment	= firstSegment;
					secondTurnSegment	= secondSegment;
				}

				//connectionPoint - point in the middle axis of a segment at its border with crossroad/turn
				FVector firstConnectionPoint;
				FVector secondConnectionPoint;
				//borderPoint - point in the "corner" of a segment at its border with crossroad/turn
				FVector firstBorderPoint;
				FVector secondBorderPoint;
				//directionPoint - point forming a direction to a neighboring crossroad with borderPoint
				FVector firstDirectionPoint;
				FVector secondDirectionPoint;

				FindSegmentToCrossroadConnection(inNetworkGeometry, crossroadId, firstSegmentId, 
					true, firstConnectionPoint, firstBorderPoint, firstDirectionPoint);
				FindSegmentToCrossroadConnection(inNetworkGeometry, crossroadId, secondSegmentId, 
					false, secondConnectionPoint, secondBorderPoint, secondDirectionPoint);

				auto arcType = crossroad.Arcs[i];
				auto arcCenter = crossroad.ArcsCenters[i];

				TArray<FVector> arcPoints;
				switch (arcType)
				{
					case ArcType::INNER_ARC:
					{
						auto turningRadius = FMath::Min(firstSegment.TurningRadius, secondSegment.TurningRadius);

						auto firstDirection		= (firstConnectionPoint		- crossroadCenter).GetSafeNormal();
						auto secondDirection	= (secondConnectionPoint	- crossroadCenter).GetSafeNormal();
						firstDirection	= FVector::CrossProduct(firstDirection,		FVector::UpVector);
						secondDirection	= FVector::CrossProduct(secondDirection,	FVector::DownVector);

						auto firstAngle		= FMath::Atan2(firstDirection.Y,	firstDirection.X);
						auto secondAngle	= FMath::Atan2(secondDirection.Y,	secondDirection.X);
						if (firstAngle < secondAngle)
						{
							firstAngle += 2 * PI;
						}
						auto fullAngle = firstAngle - secondAngle;
						auto angleStepNums = FMath::Max(FMath::RoundToInt(fullAngle / URoadBuilder::ArcsAngleStep), 1);
						auto angleStep = fullAngle / angleStepNums;

						arcPoints.Add(firstBorderPoint);
						for (int j = 0; j <= angleStepNums; j++)
						{
							auto currentAngle = firstAngle - j * angleStep;
							auto currentPoint = arcCenter 
								+ turningRadius * FVector(FMath::Cos(currentAngle), FMath::Sin(currentAngle), 0);
							arcPoints.Add(currentPoint);
						}
						arcPoints.Add(secondBorderPoint);
					}
					break;

					case ArcType::OUTER_ARC:
					{
						auto turningRadius = FMath::Min(firstSegment.Width, secondSegment.Width) / 2;

						auto firstDeltaAngle = GetDirectionAngleForOuterArc(crossroadCenter,
							firstConnectionPoint, firstBorderPoint, turningRadius);
						auto secondDeltaAngle = GetDirectionAngleForOuterArc(crossroadCenter,
							secondConnectionPoint, secondBorderPoint, turningRadius);

						auto firstDirection		= (firstConnectionPoint		- crossroadCenter).GetSafeNormal();
						auto secondDirection	= (secondConnectionPoint	- crossroadCenter).GetSafeNormal();

						auto firstAngle		= FMath::Atan2(firstDirection.Y,	firstDirection.X);
						auto secondAngle	= FMath::Atan2(secondDirection.Y,	secondDirection.X);

						firstAngle	+= firstDeltaAngle;
						secondAngle	-= secondDeltaAngle;

						if (secondAngle < firstAngle)
						{
							secondAngle += 2 * PI;
						}
						auto fullAngle = secondAngle - firstAngle;
						auto angleStepNums = FMath::Max(FMath::RoundToInt(fullAngle / URoadBuilder::ArcsAngleStep), 1);
						auto angleStep = fullAngle / angleStepNums;

						arcPoints.Add(firstBorderPoint);
						for (int j = 0; j <= angleStepNums; j++)
						{
							auto currentAngle = firstAngle + j * angleStep;
							auto currentPoint = crossroadCenter
								+ turningRadius * FVector(FMath::Cos(currentAngle), FMath::Sin(currentAngle), 0);
							arcPoints.Add(currentPoint);
						}
						arcPoints.Add(secondBorderPoint);
					}
					break;

					case ArcType::NO_ARC:
					{
						auto stepFromCrossroad = (firstSegment.Width > secondSegment.Width)
							? firstBorderPoint - firstConnectionPoint
							: secondBorderPoint - secondConnectionPoint;

						arcPoints.Add(firstBorderPoint);
						arcPoints.Add(crossroadCenter + stepFromCrossroad);
						arcPoints.Add(secondBorderPoint);
					}
					break;
				}

				mainPoints.Append(arcPoints);

				curtainsPoints.Add(firstDirectionPoint);
				curtainsPoints.Append(arcPoints);
				curtainsPoints.Add(secondDirectionPoint);

				curtainsSeparatorsIndeces.Add(curtainsPoints.Num());
			}

			//isTurn determines, among other things, whether to use segments or crossroads material
			bool isTurn = (segmentsNum == 2) && (firstTurnSegment.Lanes == secondTurnSegment.Lanes);
			auto & sectionData = isTurn ? segmentsSectionData : crossroadsSectionData;

			auto centerIndex = sectionData.Vertices.Num();

			sectionData.Vertices		.Add(crossroadCenter);
			sectionData.Uv1				.Add(FVector2D());
			sectionData.Normals			.Add(FVector::UpVector);
			sectionData.VertexColors	.Add(FColor(1, 1, 1, 1));
			sectionData.Tangents		.Add(FRuntimeMeshTangent());

			sectionData.Vertices.Append(mainPoints);
			auto pointsNum = mainPoints.Num();
			for (int j = 0; j < pointsNum; j++)
			{
				sectionData.Indices.Append({
					centerIndex + 0,
					centerIndex + (j + 1) % pointsNum + 1,
					centerIndex + j + 1,
					});

				sectionData.Uv1				.Add(FVector2D());
				sectionData.Normals			.Add(FVector::UpVector);
				sectionData.VertexColors	.Add(FColor(1, 1, 1, 1));
				sectionData.Tangents		.Add(FRuntimeMeshTangent());
			}

			//UV0 for turns/crossroads
			if (isTurn)
			{
				auto lanes = firstTurnSegment.Lanes;

				TArray<float> firstBorderRatios;
				firstBorderRatios.Add(0);
				TArray<float> secondBorderRatios;
				secondBorderRatios.Add(0);
				float firstBorderLength = 0;
				float secondBorderLength = 0;

				for (int i = 0; i < turnSeparatorPointIndex - 1; i++)
				{
					auto step = (mainPoints[i] - mainPoints[i + 1]).Size();
					firstBorderRatios.Add(firstBorderRatios.Last() + step);
					firstBorderLength += step;
				}
				for (int i = turnSeparatorPointIndex; i < mainPoints.Num() - 1; i++)
				{
					auto step = (mainPoints[i] - mainPoints[i + 1]).Size();
					secondBorderRatios.Add(secondBorderRatios.Last() + step);
					secondBorderLength += step;
				}

				auto turnLength = static_cast<float>(FMath::Max(FMath::RoundToInt((firstBorderLength + secondBorderLength) / 2
					/ (firstTurnSegment.Width * inStretch) * lanes), 1));

				//calculations for turn center's U texture coordinate
				auto secondBorderSize = mainPoints.Num() - turnSeparatorPointIndex;
				auto firstBorderCenter = (turnSeparatorPointIndex % 2 == 0)
					? (mainPoints[turnSeparatorPointIndex / 2 - 1] + mainPoints[turnSeparatorPointIndex / 2]) / 2
					: mainPoints[turnSeparatorPointIndex / 2];
				auto secondBorderCenter = (secondBorderSize % 2 == 0)
					? (mainPoints[turnSeparatorPointIndex + secondBorderSize / 2 - 1] 
						+ mainPoints[turnSeparatorPointIndex + secondBorderSize / 2]) / 2
					: mainPoints[turnSeparatorPointIndex + secondBorderSize / 2];
				auto centerToFirstBorderDistance = (firstBorderCenter - crossroadCenter).Size();
				auto centerToSecondBorderDistance = (secondBorderCenter - crossroadCenter).Size();
				auto turnCenterRatioX = centerToFirstBorderDistance / (centerToFirstBorderDistance + centerToSecondBorderDistance);

				//calculations for turn center's V texture coordinate
				auto turnStartPoint = (mainPoints[0] + mainPoints[mainPoints.Num() - 1]) / 2;
				auto turnEndPoint = (mainPoints[turnSeparatorPointIndex - 1] + mainPoints[turnSeparatorPointIndex]) / 2;
				auto centerToTurnStartDistance = (turnStartPoint - crossroadCenter).Size();
				auto centerToTurnEndDistance = (turnEndPoint - crossroadCenter).Size();
				auto turnCenterRatioY = centerToTurnStartDistance / (centerToTurnStartDistance + centerToTurnEndDistance);

				sectionData.Uv0.Add(FVector2D(turnCenterRatioX * lanes, turnCenterRatioY * turnLength));

				for (auto firstBorderRatio : firstBorderRatios)
				{
					sectionData.Uv0.Add(FVector2D(0, firstBorderRatio / firstBorderLength * turnLength));
				}
				for (auto secondBorderRatio : secondBorderRatios)
				{
					sectionData.Uv0.Add(FVector2D(lanes, (secondBorderLength - secondBorderRatio) / secondBorderLength * turnLength));
				}
			}
			//crossroad's UVs
			else 
			{
				sectionData.Uv0.Add(FVector2D(0.5f, 0.5f));
				for (int j = 0; j < pointsNum; j++)
				{
					sectionData.Uv0.Add(FVector2D(0, 0));
				}
			}

			//curtains
			for (int i = 0; i < curtainsSeparatorsIndeces.Num() - 1; i++)
			{
				TArray<float> borderRatios;
				borderRatios.Add(0);
				float borderLength = 0;

				int startIndex	= curtainsSeparatorsIndeces[i] + 1;
				int endIndex	= curtainsSeparatorsIndeces[i + 1] - 2;
				int startVertexIndex = outCurtainsMeshData.Vertices.Num();
				for (int j = startIndex; j <= endIndex; j++)
				{
					auto currentPoint	= curtainsPoints[j];
					auto prevPoint		= curtainsPoints[j - 1];
					auto nextPoint		= curtainsPoints[j + 1];

					auto pointDelta = URoadBuilder::CalculatePerpendicularToLine(nextPoint, prevPoint);
					auto curtainsDelta = pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);

					outCurtainsMeshData.Vertices		.Add(currentPoint);
					outCurtainsMeshData.Vertices		.Add(currentPoint + curtainsDelta);
					outCurtainsMeshData.Uv1				.Append({ LIST_2_TIMES(FVector2D()) });
					outCurtainsMeshData.Normals			.Append({ LIST_2_TIMES(FVector::UpVector) });
					outCurtainsMeshData.VertexColors	.Append({ LIST_2_TIMES(FColor(1, 1, 1, 1)) });
					outCurtainsMeshData.Tangents		.Append({ LIST_2_TIMES(FRuntimeMeshTangent()) });

					if (j != endIndex)
					{
						auto indicesDelta = startVertexIndex + 2 * (j - startIndex);
						outCurtainsMeshData.Indices.Append({
								0 + indicesDelta,
								2 + indicesDelta,
								1 + indicesDelta,
								3 + indicesDelta,
								1 + indicesDelta,
								2 + indicesDelta,
							});

						auto step = (curtainsPoints[j] - curtainsPoints[j + 1]).Size();
						borderRatios.Add(borderRatios.Last() + step);
						borderLength += step;
					}
				}

				for (auto borderRatio : borderRatios)
				{
					outCurtainsMeshData.Uv0.Add(FVector2D(0, borderRatio / borderLength));
					outCurtainsMeshData.Uv0.Add(FVector2D(1, borderRatio / borderLength));
				}
			}

		}
		//dead end
		else if (segmentsNum == 1)
		{
			auto segment = inNetworkGeometry.Segments[crossroad.SegmentsIds[0]];

			int otherCrossroadId = (crossroadId == segment.StartCrossroadId)
				? segment.EndCrossroadId
				: segment.StartCrossroadId;

			auto otherPoint = inNetworkGeometry.Crossroads[otherCrossroadId].Center;
			auto direction = (otherPoint - crossroadCenter).GetSafeNormal();
			auto delta = segment.Width / 2 * FVector::CrossProduct(FVector::UpVector, direction);
			auto point = crossroadCenter;

			auto indicesDelta = segmentsSectionData.Vertices.Num();
			auto indicesDeltaCurtains = outCurtainsMeshData.Vertices.Num();

			segmentsSectionData.Vertices		.Add(point);
			segmentsSectionData.Uv0				.Add(FVector2D(static_cast<float>(segment.Lanes) / 2, 0));
			segmentsSectionData.Uv1				.Add(FVector2D());
			segmentsSectionData.Normals			.Add(FVector::UpVector);
			segmentsSectionData.VertexColors	.Add(FColor::White);
			segmentsSectionData.Tangents		.Add(FRuntimeMeshTangent());

			int capDensity = FMath::RoundToInt(PI / URoadBuilder::ArcsAngleStep);
			TArray<FVector2D> uvs;
			TArray<FVector2D> cupsPointsDirections = URoadBuilder::GetRoadCupsPointsDirections(capDensity);
			TArray<FVector> radiusDeltas = URoadBuilder::GetCupsPointsOffsets(cupsPointsDirections, delta, false);

			for (int j = 0; j <= capDensity; j++)
			{
				auto radiusDelta = radiusDeltas[j];
				auto size = radiusDelta.Size();
				auto curtainsDelta = radiusDelta / size * inCurtainsWidth - FVector(0, 0, inRoadHeight);

				auto uv0Y = static_cast<float>(j) / capDensity * segment.Lanes;

				segmentsSectionData.Vertices		.Add(point + radiusDelta);
				segmentsSectionData.Uv0				.Add(FVector2D(0, uv0Y));
				segmentsSectionData.Uv1				.Add(FVector2D());
				segmentsSectionData.Normals			.Add(FVector::UpVector);
				segmentsSectionData.VertexColors	.Add(FColor::White);
				segmentsSectionData.Tangents		.Add(FRuntimeMeshTangent());

				outCurtainsMeshData.Vertices		.Add(point + radiusDelta);
				outCurtainsMeshData.Vertices		.Add(point + radiusDelta + curtainsDelta);
				outCurtainsMeshData.Uv0				.Add(FVector2D(0, uv0Y));
				outCurtainsMeshData.Uv0				.Add(FVector2D(1, uv0Y));
				outCurtainsMeshData.Uv1				.Append({ LIST_2_TIMES(FVector2D()) });
				outCurtainsMeshData.Normals			.Append({ LIST_2_TIMES(FVector::UpVector) });
				outCurtainsMeshData.VertexColors	.Append({ LIST_2_TIMES(FColor::White) });
				outCurtainsMeshData.Tangents		.Append({ LIST_2_TIMES(FRuntimeMeshTangent()) });

				if (j > 0)
				{
					segmentsSectionData.Indices.Append({ indicesDelta, indicesDelta + j, indicesDelta + j + 1 });
					auto baseIndex = indicesDeltaCurtains + 2 * j;
					outCurtainsMeshData.Indices.Append({
						baseIndex - 2,
						baseIndex - 1,
						baseIndex,
						baseIndex + 1,
						baseIndex,
						baseIndex - 1
						});
				}
			}
		}
	}

	return TPair<MeshSectionData, MeshSectionData>(segmentsSectionData, crossroadsSectionData);
}


void URoadBuilder::ConstructRoadMeshSection(URuntimeMeshComponent* inRuntimeMesh, RoadNetworkGeometry inNetworkGeometry,
	int inSegmentsSectionIndex, int inCrossroadsSectionIndex,
	UMaterialInstanceDynamic* inSegmentsMaterial, UMaterialInstanceDynamic* inCrossroadsMaterial, 
	MeshSectionData& outCurtainsMeshData)
{
	auto sectionsData = CalculateMeshDataForRoad(inNetworkGeometry, outCurtainsMeshData,
		RoadHeight, CurtainsWidth, Stretch);
	auto segmentsSectionData	= sectionsData.Key;
	auto crossroadsSectionData	= sectionsData.Value;

	inRuntimeMesh->CreateMeshSection(inSegmentsSectionIndex, segmentsSectionData.Vertices, segmentsSectionData.Indices,
		segmentsSectionData.Normals, segmentsSectionData.Uv0, segmentsSectionData.Uv1, segmentsSectionData.VertexColors, 
		segmentsSectionData.Tangents, false);

	inRuntimeMesh->CreateMeshSection(inCrossroadsSectionIndex, crossroadsSectionData.Vertices, crossroadsSectionData.Indices,
		crossroadsSectionData.Normals, crossroadsSectionData.Uv0, crossroadsSectionData.Uv1, crossroadsSectionData.VertexColors,
		crossroadsSectionData.Tangents, false);

	inRuntimeMesh->SetMaterial(inSegmentsSectionIndex,		inSegmentsMaterial);
	inRuntimeMesh->SetMaterial(inCrossroadsSectionIndex,	inCrossroadsMaterial);
}


void URoadBuilder::SpawnRoadNetworkActor(FRoadNetwork inRoadNetwork)
{
	if (roadNetworkActor)
	{
		RemoveRoadNetworkActor();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Name = "RoadNetworkActor";
	roadNetworkActor = GetWorld()->SpawnActor<ARoadNetworkActor>(FVector(0, 0, AutoRoadZ), FRotator::ZeroRotator, SpawnInfo);
	roadNetworkActor->SetActorLabel(SpawnInfo.Name.ToString());
	roadNetworkActor->SetMobility(EComponentMobility::Movable);
	auto runtimeMesh = roadNetworkActor->GetRuntimeMeshComponent();

	const int CURTAINS_MATERIAL_INDEX	= 0;
	const int SEGMENTS_MATERIAL_INDEX	= 1;
	const int CROSSROADS_MATERIAL_INDEX	= 2;
	TMap<int, UMaterialInstanceDynamic*> roadMaterials = {
		{ CURTAINS_MATERIAL_INDEX,		UMaterialInstanceDynamic::Create(CurtainsMaterial,		this) },
		{ SEGMENTS_MATERIAL_INDEX,		UMaterialInstanceDynamic::Create(RoadSegmentsMaterial,	this) },
		{ CROSSROADS_MATERIAL_INDEX,	UMaterialInstanceDynamic::Create(CrossroadsMaterial,	this) },
	};

	auto geometry = GetRoadNetworkGeometry(inRoadNetwork);
	MeshSectionData curtainsMeshData;

	ConstructRoadMeshSection(runtimeMesh, geometry, SEGMENTS_MATERIAL_INDEX, CROSSROADS_MATERIAL_INDEX,
		roadMaterials[SEGMENTS_MATERIAL_INDEX], roadMaterials[CROSSROADS_MATERIAL_INDEX], curtainsMeshData);

	runtimeMesh->CreateMeshSection(CURTAINS_MATERIAL_INDEX, curtainsMeshData.Vertices, curtainsMeshData.Indices, curtainsMeshData.Normals,
		curtainsMeshData.Uv0, curtainsMeshData.Uv1, curtainsMeshData.VertexColors, curtainsMeshData.Tangents, false);
	runtimeMesh->SetMaterial(CURTAINS_MATERIAL_INDEX, roadMaterials[CURTAINS_MATERIAL_INDEX]);

	roadNetworkActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
}