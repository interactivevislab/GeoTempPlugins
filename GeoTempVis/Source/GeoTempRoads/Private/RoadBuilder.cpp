#include "RoadBuilder.h"

#include "RoadNetworkActor.h"

#include "DrawDebugHelpers.h"


#define LIST_4_TIMES(something) something, something, something, something
#define LIST_8_TIMES(something) LIST_4_TIMES(something), LIST_4_TIMES(something)


struct RoadSegmentGeometry
{
	int StartCrossroadId;
	int EndCrossroadId;
	int Lanes;
	float Width;
	float TurningRadius;
	float DistanceFromStartCrossroad = 0;
	float DistanceFromEndCrossroad = 0;
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

			if (isSpecialCase)
			{
				crossroad.Arcs.Add(ArcType::NO_ARC);
				crossroad.ArcsCenters.Add(FVector());
			}
			else if (angle >= PI)
			{
				isSpecialCase = true;
				crossroad.Arcs.Add(ArcType::OUTER_ARC);
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
		
		if (needToEmplace)
		{
			convertedGeometry.Segments.Emplace(segmentId, segment);
		}

		auto length = (convertedGeometry.Crossroads[segment.StartCrossroadId].Center 
			- convertedGeometry.Crossroads[segment.EndCrossroadId].Center).Size();
		if (segment.DistanceFromStartCrossroad + segment.DistanceFromEndCrossroad > length)
		{
			// invalid segment distances (to close to crossroads)
		}
	}

	return convertedGeometry;
}


#pragma region OldCode

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


MeshSectionData CalculateMeshDataForRoad(TArray<FRoadSegment> inSegments, MeshSectionData& outCurtainsMeshData,
	float inAutoRoadZ, float inRailRoadZ, float inRoadHeight, float inCurtainsWidth, float inStretch)
{
	MeshSectionData sectionData;

	for (auto segment : inSegments)
	{
		auto roadZ = (segment.Type == EHighwayType::Auto) ? inAutoRoadZ : inRailRoadZ;

		for (int i = 0; i < segment.AllPoints.Num() - 1; i++)
		{
			auto startPoint	= segment.AllPoints[i];
			auto endPoint	= segment.AllPoints[i + 1];
			startPoint.Z	= roadZ;
			endPoint.Z		= roadZ - 1;

			auto pointDelta = URoadBuilder::CalculatePerpendicularToLine(startPoint, endPoint);
			auto curtainsDelta1 =	 pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			auto curtainsDelta2 =	-pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			
			pointDelta *= (segment.Width * 50);
			
			auto roadRectangle = URoadBuilder::ConvertLineToRect(startPoint, endPoint, pointDelta);

			auto indicesDelta = sectionData.Vertices.Num();
			sectionData.Vertices.Append(roadRectangle);

			sectionData.Indices.Append({
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

			auto curtainsNormal1 = FVector::UpVector;
			auto curtainsNormal2 = FVector::UpVector;

			auto lenght = FMath::RoundToInt((startPoint - endPoint).Size() 
				/ (segment.Width * 100 * inStretch) * segment.Lanes);
			
			auto uv00	= FVector2D(			0,		0);
			auto uv01	= FVector2D(segment.Lanes,		0);
			auto uv01c	= FVector2D(			1,		0);
			auto uv02	= FVector2D(			0, lenght);
			auto uv03	= FVector2D(segment.Lanes, lenght);
			auto uv03c	= FVector2D(			1, lenght);

			sectionData.Uv0					.Append({ uv00, uv01, uv02, uv03 });			
			sectionData.Uv1					.Append({ LIST_4_TIMES(FVector2D()) });
			sectionData.Normals				.Append({ LIST_4_TIMES(FVector::UpVector) });
			sectionData.VertexColors		.Append({ LIST_4_TIMES(FColor(1, 1, 1, 1)) });
			sectionData.Tangents			.Append({ LIST_4_TIMES(FRuntimeMeshTangent()) });
			
			outCurtainsMeshData.Uv0			.Append({ uv00, uv01c, uv02, uv03c, uv00, uv01c, uv02, uv03c });
			outCurtainsMeshData.Uv1			.Append({ LIST_8_TIMES(FVector2D()) });			
			outCurtainsMeshData.Normals		.Append({ LIST_4_TIMES(curtainsNormal1), LIST_4_TIMES(curtainsNormal2) });			
			outCurtainsMeshData.VertexColors.Append({ LIST_8_TIMES(FColor(1, 1, 1, 1)) });			
			outCurtainsMeshData.Tangents	.Append({ LIST_8_TIMES(FRuntimeMeshTangent()) });

			for (auto point : { endPoint, startPoint })
			{
				bool isReversedCup = (point == endPoint);

				point -= FVector(0, 0, 2);
				
				indicesDelta				= sectionData.Vertices.Num();
				auto indicesDelta_curtains	= outCurtainsMeshData.Vertices.Num();
				
				sectionData.Vertices	.Add(point);
				sectionData.Uv0			.Add(FVector2D(0.5 * segment.Lanes, 0.5));
				sectionData.Uv1			.Add(FVector2D());
				sectionData.Normals		.Add(FVector::UpVector);
				sectionData.VertexColors.Add(FColor::White);
				sectionData.Tangents	.Add(FRuntimeMeshTangent());

				
				TArray<FVector2D>	uvs;		
				TArray<FVector2D>	uvs_curtains = URoadBuilder::GetRoadCupsPointsDirections();
				TArray<FVector>		radiusDeltas = URoadBuilder::GetCupsPointsOffsets(uvs_curtains, pointDelta, isReversedCup);

				for (int j = 0; j <= URoadBuilder::capDensity; j++)
				{
					uvs.Add(FVector2D(0, static_cast<float>(j) / URoadBuilder::capDensity * segment.Lanes));

					auto radiusDelta	= radiusDeltas[j];
					auto size			= radiusDelta.Size();
					auto curtainsDelta	= radiusDelta / size * inCurtainsWidth - FVector(0, 0, inRoadHeight);
					auto curtainNormal	= FVector::CrossProduct(
						(radiusDeltas[FMath::Max(j - 1, 0)] - radiusDelta).GetSafeNormal(), 
						(-curtainsDelta).GetSafeNormal());
					
					
					sectionData.Vertices	.Add(point + radiusDelta);
					sectionData.Uv0			.Add(uvs[j]);
					sectionData.Uv1			.Add(FVector2D());
					sectionData.Normals		.Add(FVector::UpVector);
					sectionData.VertexColors.Add(FColor::White);
					sectionData.Tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.Vertices	.Add(point + radiusDelta);
					outCurtainsMeshData.Uv0			.Add(uvs_curtains[j]);
					outCurtainsMeshData.Uv1			.Add(FVector2D());
					outCurtainsMeshData.Normals		.Add(curtainNormal);
					outCurtainsMeshData.VertexColors.Add(FColor::White);
					outCurtainsMeshData.Tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.Vertices	.Add(point + radiusDelta + curtainsDelta);
					outCurtainsMeshData.Uv0			.Add(2 * uvs_curtains[j]);
					outCurtainsMeshData.Uv1			.Add(FVector2D());
					outCurtainsMeshData.Normals		.Add(curtainNormal);
					outCurtainsMeshData.VertexColors.Add(FColor::White);
					outCurtainsMeshData.Tangents	.Add(FRuntimeMeshTangent());

					if (j > 0)
					{
						sectionData.Indices.Append({ indicesDelta, indicesDelta + j, indicesDelta + j + 1 });
						auto baseIndex = indicesDelta_curtains + 2 * j;
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
	}
	return sectionData;
}


void URoadBuilder::ConstructRoadMeshSection(URuntimeMeshComponent* inRuntimeMesh, TArray<FRoadSegment> inSegments,
	int inSectionIndex, UMaterialInstanceDynamic* inMaterial, MeshSectionData& outCurtainsMeshData)
{
	auto sectionData = CalculateMeshDataForRoad(inSegments, outCurtainsMeshData, 
		AutoRoadZ, RailRoadZ, RoadHeight, CurtainsWidth, Stretch);
	if(sectionData.Indices.Num() == 0 || sectionData.Vertices.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Attempt to create an empty mesh"));
		return;
	}
	inRuntimeMesh->CreateMeshSection(inSectionIndex, sectionData.Vertices, sectionData.Indices, sectionData.Normals,
		sectionData.Uv0, sectionData.Uv1, sectionData.VertexColors, sectionData.Tangents, false);
	inRuntimeMesh->SetMaterial(inSectionIndex, inMaterial);
}


void URoadBuilder::SpawnRoadNetworkActor(FRoadNetwork inRoadNetwork)
{
	const int CURTAINS_MATERIAL_INDEX	= 0;
	const int AUTO_MATERIAL_INDEX		= 1;
	const int RAIL_MATERIAL_INDEX		= 2;

	TMap<int, UMaterialInstanceDynamic*> roadMaterials = {
		{ CURTAINS_MATERIAL_INDEX,	UMaterialInstanceDynamic::Create(RoadMaterial, this) },
		{ AUTO_MATERIAL_INDEX,		UMaterialInstanceDynamic::Create(RoadMaterial, this) },
		{ RAIL_MATERIAL_INDEX,		UMaterialInstanceDynamic::Create(RoadMaterial, this) },
	};

	roadMaterials[CURTAINS_MATERIAL_INDEX]	->SetScalarParameterValue("CoatingType", static_cast<float>(RoadType::Sand));
	roadMaterials[AUTO_MATERIAL_INDEX]		->SetScalarParameterValue("CoatingType", static_cast<float>(RoadType::Asphalt));
	roadMaterials[RAIL_MATERIAL_INDEX]		->SetScalarParameterValue("CoatingType", static_cast<float>(RoadType::Rail));

	TArray<FRoadSegment> autoSegments, railSegments;
	for (auto segmentData : inRoadNetwork.Segments)
	{
		auto segment = segmentData.Value;
		switch (segment.Type)
		{
			case EHighwayType::Auto:
			{
				autoSegments.Add(segment);
			}
			break;

			case EHighwayType::Rail:
			{
				railSegments.Add(segment);
			}
			break;
		}
	}

	if (roadNetworkActor)
	{
		RemoveRoadNetworkActor();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Name = "RoadNetworkActor";
	roadNetworkActor = GetWorld()->SpawnActor<ARoadNetworkActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	roadNetworkActor->SetActorLabel(SpawnInfo.Name.ToString());
	roadNetworkActor->SetMobility(EComponentMobility::Movable);
	auto runtimeMesh = roadNetworkActor->GetRuntimeMeshComponent();

	MeshSectionData curtainsMeshData;
	ConstructRoadMeshSection(runtimeMesh, autoSegments, AUTO_MATERIAL_INDEX, 
		roadMaterials[AUTO_MATERIAL_INDEX], curtainsMeshData);
	ConstructRoadMeshSection(runtimeMesh, railSegments, RAIL_MATERIAL_INDEX, 
		roadMaterials[RAIL_MATERIAL_INDEX], curtainsMeshData);

	if(curtainsMeshData.Indices.Num() == 0 || curtainsMeshData.Vertices.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Attempt to create an empty mesh"));
		return;
	}
	runtimeMesh->CreateMeshSection(CURTAINS_MATERIAL_INDEX, curtainsMeshData.Vertices, curtainsMeshData.Indices, curtainsMeshData.Normals,
		curtainsMeshData.Uv0, curtainsMeshData.Uv1, curtainsMeshData.VertexColors, curtainsMeshData.Tangents, false);
	runtimeMesh->SetMaterial(CURTAINS_MATERIAL_INDEX, roadMaterials[CURTAINS_MATERIAL_INDEX]);

	roadNetworkActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
}


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


TArray<FVector2D> URoadBuilder::GetRoadCupsPointsDirections()
{
	TArray<FVector2D> pointsDirections = {};
	for (int j = 0; j <= URoadBuilder::capDensity; j++)
	{
		float angle = PI / URoadBuilder::capDensity * j;

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

#pragma endregion


void URoadBuilder::SpawnNewRoadNetworkActor(FRoadNetwork inRoadNetwork)
{
	if (roadNetworkActor)
	{
		RemoveRoadNetworkActor();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Name = "RoadNetworkActor";
	roadNetworkActor = GetWorld()->SpawnActor<ARoadNetworkActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	roadNetworkActor->SetActorLabel(SpawnInfo.Name.ToString());
	roadNetworkActor->SetMobility(EComponentMobility::Movable);
	auto runtimeMesh = roadNetworkActor->GetRuntimeMeshComponent();

	//

	auto geometry = GetRoadNetworkGeometry(inRoadNetwork);
	auto world = GetWorld();
	float debugDuration = 120;

	for (auto crossroadData : geometry.Crossroads)
	{
		auto crossroadId = crossroadData.Key;
		auto crossroad = crossroadData.Value;
		DrawDebugBox(world, crossroad.Center, FVector(100), FColor::Blue, false, debugDuration, 0, 50);
		for (int i = 0; i < crossroad.Arcs.Num(); i++)
		{
			if (crossroad.Arcs[i] == ArcType::INNER_ARC)
			{
				DrawDebugBox(world, crossroad.ArcsCenters[i], FVector(50), FColor::Cyan, false, debugDuration, 0, 25);
				DrawDebugLine(world, crossroad.ArcsCenters[i], crossroad.Center, FColor::Cyan, false, debugDuration, 0, 20);
			}
		}
	}

	for (auto segmentData : geometry.Segments)
	{
		auto segmentId = segmentData.Key;
		auto segment = segmentData.Value;

		auto startPoint = geometry.Crossroads[segment.StartCrossroadId].Center;
		auto endPoint = geometry.Crossroads[segment.EndCrossroadId].Center;

		auto direction = (endPoint - startPoint).GetSafeNormal();
		auto length = (endPoint - startPoint).Size();

		if (length < segment.DistanceFromStartCrossroad + segment.DistanceFromEndCrossroad)
		{
			DrawDebugLine(world, startPoint, endPoint, FColor::Red, false, debugDuration, 0, 50);
		}
		else
		{
			startPoint += segment.DistanceFromStartCrossroad * direction;
			endPoint -= segment.DistanceFromEndCrossroad * direction;
			DrawDebugLine(world, startPoint, endPoint, FColor::Green, false, debugDuration, 0, 100);
		}
	}

	//

	roadNetworkActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
}