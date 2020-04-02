#include "RoadBuilder.h"

#include "Dom/JsonObject.h"
#pragma warning( disable : 4456)

#define LIST_3_TIMES(something) something, something, something
#define LIST_4_TIMES(something) something, something, something, something
#define LIST_8_TIMES(something) LIST_4_TIMES(something), LIST_4_TIMES(something)
#define LIST_12_TIMES(something) LIST_3_TIMES(LIST_4_TIMES(something))


FRoadNetwork URoadBuilder::ProcessRoadNetwork(FPostGisRoadNetwork inApiRoadNetwork)
{
	TMap<int, FRoadSegment> segments;
	TMap<int, FCrossroad>	crossroads;
	TMap<FVector, int>		crossroadIds;
	
	int nextSegmentId	= 0;
	int nextCrossroadId = 0;
	
	for (auto apiSegmentPair : inApiRoadNetwork.Segments)
	{
		auto apiSegment = apiSegmentPair.Value;

		FRoadSegment segment;
		segment.Type		= apiSegment.Highway;
		segment.Width		= apiSegment.Lanes * apiSegment.LaneWidth;
		segment.Lanes		= apiSegment.Lanes;
		segment.StartYear	= apiSegment.YearStart;
		segment.EndYear		= apiSegment.YearEnd;
		segment.Change		= apiSegment.Change;
		
		auto pointStart	= apiSegment.Line.Start;
		auto pointEnd		= apiSegment.Line.End;

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

		segment.AllPoints = apiSegmentPair.Value.Line.AllPoints;

		crossroadStart	->Roads.Add(nextSegmentId, segment.EndCrossroadId);
		crossroadEnd	->Roads.Add(nextSegmentId, segment.StartCrossroadId);
		
		segments.Add(nextSegmentId++, segment);
	}

	return FRoadNetwork { segments, crossroads };
}


FRoadNetwork URoadBuilder::GetRoadNetworkForYear(FRoadNetwork inFullRoadNetwork, int inYear)
{
	TMap<int, FRoadSegment>	segments;
	TMap<int, FCrossroad>	crossroads;
	TSet<int>				crossroadsIds;
	
	for (auto segmentData : inFullRoadNetwork.Segments) {
		auto segment = segmentData.Value;
		if ((segment.StartYear <= inYear) && (segment.EndYear > inYear)) {
			segments		.Add(segmentData);
			crossroadsIds	.Add(segment.StartCrossroadId);
			crossroadsIds	.Add(segment.EndCrossroadId);
		}
	}
	for (auto crossroadId : crossroadsIds) {
		auto crossroad = *(inFullRoadNetwork.Crossroads.Find(crossroadId));
		crossroads.Add(crossroadId, crossroad);
	}
	return FRoadNetwork{ segments, crossroads };
}


MeshSectionData CalculateMeshDataForRoad(TArray<FRoadSegment> inSegments, MeshSectionData& outCurtainsMeshData,
	float inAutoRoadZ, float inRailRoadZ, float inRoadHeight, float inCurtainsWidth, float inStretch)
{
	MeshSectionData sectionData;

	for (auto segment : inSegments)
	{
		auto uv1Data = FVector2D(segment.StartYear, segment.EndYear);

		auto roadZ = (segment.Type == EHighwayType::Auto) ? inAutoRoadZ : inRailRoadZ;

		for (int i = 0; i < segment.AllPoints.Num() - 1; i++)
		{
			auto startPoint	= segment.AllPoints[i];
			auto endPoint		= segment.AllPoints[i + 1];
			startPoint.Z	= roadZ;
			endPoint.Z		= roadZ - 1;

			auto pointDelta = FVector::CrossProduct((startPoint - endPoint).GetSafeNormal(), FVector(0, 0, 1));
			auto curtainsDelta1 =	 pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			auto curtainsDelta2 =	-pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			
			pointDelta *= (segment.Width * 50);
			
			auto point0 = startPoint	+ pointDelta;
			auto point1 = startPoint	- pointDelta;
			auto point2 = endPoint	+ pointDelta;
			auto point3 = endPoint	- pointDelta;

			auto indicesDelta = sectionData.vertices.Num();
			sectionData.vertices.Append({
				point0,
				point1,
				point2,
				point3,
			});

			sectionData.indices.Append({
				0 + indicesDelta,
				2 + indicesDelta,
				1 + indicesDelta,
				3 + indicesDelta,
				1 + indicesDelta,
				2 + indicesDelta,
			});

			indicesDelta = outCurtainsMeshData.vertices.Num();
			outCurtainsMeshData.vertices.Append({
				point0 + curtainsDelta1,
				point0,
				point2 + curtainsDelta1,
				point2,

				point1,
				point1 + curtainsDelta2,
				point3,
				point3 + curtainsDelta2
			});

			outCurtainsMeshData.indices.Append({
				0 + indicesDelta,
				2 + indicesDelta,
				1 + indicesDelta,
				3 + indicesDelta,
				1 + indicesDelta,
				2 + indicesDelta,
			});
			indicesDelta += 4;
			outCurtainsMeshData.indices.Append({
				0 + indicesDelta,
				2 + indicesDelta,
				1 + indicesDelta,
				3 + indicesDelta,
				1 + indicesDelta,
				2 + indicesDelta,
			});

			auto curtainsNormal1 = FVector::UpVector;
			auto curtainsNormal2 = FVector::UpVector;

			auto lenght = FMath::RoundToInt((startPoint - endPoint).Size() / (segment.Width * 100 * inStretch) * segment.Lanes);
			
			auto uv00	= FVector2D(0, 0);
			auto uv01	= FVector2D(segment.Lanes, 0);
			auto uv01c	= FVector2D(1, 0);
			auto uv02	= FVector2D(0, lenght);
			auto uv03	= FVector2D(segment.Lanes, lenght);
			auto uv03c	= FVector2D(1, lenght);

			sectionData.uv0					.Append({ uv00, uv01, uv02, uv03 });			
			sectionData.uv1					.Append({ LIST_4_TIMES(uv1Data) });
			sectionData.normales			.Append({ LIST_4_TIMES(FVector::UpVector) });
			sectionData.vertexColors		.Append({ LIST_4_TIMES(FColor(1, 1, 1, 1)) });
			sectionData.tangents			.Append({ LIST_4_TIMES(FRuntimeMeshTangent()) });
			
			outCurtainsMeshData.uv0			.Append({ uv00, uv01c, uv02, uv03c, uv00, uv01c, uv02, uv03c });
			outCurtainsMeshData.uv1			.Append({ LIST_8_TIMES(uv1Data) });			
			outCurtainsMeshData.normales	.Append({ LIST_4_TIMES(curtainsNormal1), LIST_4_TIMES(curtainsNormal2) });			
			outCurtainsMeshData.vertexColors.Append({ LIST_8_TIMES(FColor(1, 1, 1, 1)) });			
			outCurtainsMeshData.tangents	.Append({ LIST_8_TIMES(FRuntimeMeshTangent()) });

			for (auto point : { startPoint, endPoint })
			{
				bool isReversedCup = (point == endPoint);
				const int capDensity = 8;
				auto yDelta = FVector::CrossProduct(pointDelta, FVector::UpVector);
				point -= FVector(0, 0, 2);
				
				auto indicesDelta			= sectionData.vertices.Num();
				auto indicesDelta_curtains	= outCurtainsMeshData.vertices.Num();
				
				sectionData.vertices	.Add(point);
				sectionData.uv0			.Add(FVector2D(0.5 * segment.Lanes, 0.5));
				sectionData.uv1			.Add(uv1Data);
				sectionData.normales	.Add(FVector::UpVector);
				sectionData.vertexColors.Add(FColor::White);
				sectionData.tangents	.Add(FRuntimeMeshTangent());

				FVector radiusDeltas[capDensity + 1];
				FVector2D uvs[capDensity + 1];
				FVector2D uvs_curtains[capDensity + 1];

				for (int j = 0; j <= capDensity; j++) {
					float angle = PI / capDensity * j;

					float x = FMath::Cos(angle);
					float y = FMath::Sin(angle);

					radiusDeltas[j]	= (isReversedCup ? 1 : -1) * (pointDelta * x + yDelta * y);
					uvs[j]			= FVector2D(0, (float)j / capDensity * segment.Lanes);
					uvs_curtains[j]	= FVector2D(x, y);
				}

				for (int j = 0; j <= capDensity; j++)
				{
					auto radiusDelta		= radiusDeltas[j];
					auto size				= radiusDelta.Size();
					auto curtainsDelta	= radiusDelta / size * inCurtainsWidth - FVector(0, 0, inRoadHeight);
					auto curtainNormal	= FVector::CrossProduct((radiusDeltas[FMath::Max(j - 1, 0)] - radiusDelta).GetSafeNormal(), (-curtainsDelta).GetSafeNormal());
					
					
					sectionData.vertices	.Add(point + radiusDelta);
					sectionData.uv0			.Add(uvs[j]);
					sectionData.uv1			.Add(uv1Data);
					sectionData.normales	.Add(FVector::UpVector);
					sectionData.vertexColors.Add(FColor::White);
					sectionData.tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.vertices	.Add(point + radiusDelta);
					outCurtainsMeshData.uv0			.Add(uvs_curtains[j]);
					outCurtainsMeshData.uv1			.Add(uv1Data);
					outCurtainsMeshData.normales	.Add(curtainNormal);
					outCurtainsMeshData.vertexColors.Add(FColor::White);
					outCurtainsMeshData.tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.vertices	.Add(point + radiusDelta + curtainsDelta);
					outCurtainsMeshData.uv0			.Add(2 * uvs_curtains[j]);
					outCurtainsMeshData.uv1			.Add(uv1Data);
					outCurtainsMeshData.normales	.Add(curtainNormal);
					outCurtainsMeshData.vertexColors.Add(FColor::White);
					outCurtainsMeshData.tangents	.Add(FRuntimeMeshTangent());

					if (j > 0)
					{
						sectionData.indices.Append({ indicesDelta, indicesDelta + j, indicesDelta + j + 1 });
						auto baseIndex = indicesDelta_curtains + 2 * j;
						outCurtainsMeshData.indices.Append({ baseIndex - 2, baseIndex - 1, baseIndex,
													baseIndex + 1, baseIndex, baseIndex - 1 });
					}
				}
			}
		}
	}

	return sectionData;
}


inline void URoadBuilder::ConstructRoadMeshSection(TArray<FRoadSegment> inSegments, int inSectionIndex, 
	UMaterialInstanceDynamic* inMaterial, MeshSectionData& outCurtainsMeshData)
{
	auto sectionData = CalculateMeshDataForRoad(inSegments, outCurtainsMeshData, AutoRoadZ, RailRoadZ, RoadHeight, CurtainsWidth, Stretch);

	CreateMeshSection(inSectionIndex, sectionData.vertices, sectionData.indices, sectionData.normales,
		sectionData.uv0, sectionData.uv1, sectionData.vertexColors, sectionData.tangents, false);
	SetMaterial(inSectionIndex, inMaterial);
}


void URoadBuilder::AddRoadNetworkToMesh(FRoadNetwork inRoadNetwork)
{
	roadMaterials = {
		{ CURTAINS_MATERIAL_INDEX,	UMaterialInstanceDynamic::Create(RoadMaterial, this) },
		{ RAIL_MATERIAL_INDEX,		UMaterialInstanceDynamic::Create(RoadMaterial, this) },
	};

	auto startIndex = roadMaterials.Num();
	auto endIndex = startIndex + CoatingChangeYearEnd - CoatingChangeYearStart;
	for (int i = startIndex; i <= endIndex; i++) {
		roadMaterials.Add(i, UMaterialInstanceDynamic::Create(RoadMaterial, this));
		coatingChangeDatas.Add({
			i, RoadType::Asphalt, CoatingChangeYearStart + i - startIndex
		});
	}

	roadMaterials[RAIL_MATERIAL_INDEX]->SetScalarParameterValue("CoatingType", (float)RoadType::Rail);
	roadMaterials[CURTAINS_MATERIAL_INDEX]->SetScalarParameterValue("CoatingType", (float)RoadType::Sand);

	TArray<FRoadSegment> roadSegments, railSegments, specialSegments;
	for (auto segmentData : inRoadNetwork.Segments)
	{
		auto segment = segmentData.Value;
		if (segment.Type == EHighwayType::Auto)
		{
			if (segment.Change == "") roadSegments.Add(segment);
			else specialSegments.Add(segment);
		}
		else railSegments.Add(segment);
	}

	MeshSectionData curtainsMeshData;

	ConstructRoadMeshSection(railSegments, RAIL_MATERIAL_INDEX, roadMaterials[RAIL_MATERIAL_INDEX], curtainsMeshData);

	auto numSections = endIndex - startIndex + 1;
	auto segmentsOnSection = roadSegments.Num() / numSections	+ 1;
	for (int i = 0; i < numSections; i++)
	{
		TArray<FRoadSegment> sectionSegments;
		for (int j = 0; j < segmentsOnSection; j++) {
			auto index = i + j * numSections;
			if (index < roadSegments.Num()) sectionSegments.Add(roadSegments[index]);
		}

		ConstructRoadMeshSection(sectionSegments, startIndex + i, roadMaterials[startIndex + i], curtainsMeshData);
	}

	auto startSpecialIndex = roadMaterials.Num();
	for (int i = 0; i < specialSegments.Num(); i++)
	{
		auto segment = specialSegments[i];
		auto specialIndex = startSpecialIndex + i;

		TArray<FString> changeData;
		segment.Change.ParseIntoArray(changeData, TEXT(";"));
		auto changeYear = FCString::Atoi(*changeData[0]);
		auto changeType = CoatingTags.Find(changeData[1]);

		roadMaterials.Add(specialIndex, UMaterialInstanceDynamic::Create(RoadMaterial, this));
		coatingChangeDatas.Add({
			specialIndex, (changeType != nullptr) ? *changeType : RoadType::Dirt1, changeYear
		});
		ConstructRoadMeshSection(TArray<FRoadSegment>{ segment }, specialIndex, roadMaterials[specialIndex], curtainsMeshData);
	}

	CreateMeshSection(CURTAINS_MATERIAL_INDEX, curtainsMeshData.vertices, curtainsMeshData.indices, curtainsMeshData.normales, 
		curtainsMeshData.uv0, curtainsMeshData.uv1, curtainsMeshData.vertexColors, curtainsMeshData.tangents, false);
	SetMaterial(CURTAINS_MATERIAL_INDEX, roadMaterials[CURTAINS_MATERIAL_INDEX]);
}


void URoadBuilder::SetYear(int inYear)
{
	for (auto material : roadMaterials) {
		material.Value->SetScalarParameterValue("Year", inYear);
	}
	for (auto data : coatingChangeDatas) {
		roadMaterials[data.MaterialIndex]->SetScalarParameterValue("CoatingType", (float)(inYear >= data.ChangeYear ? data.TargetCoating : RoadType::Dirt1));
	}
}


void URoadBuilder::UpdateLandscapeData(FVector4 inRect) {
	for (auto material : roadMaterials) {
		material.Value->SetScalarParameterValue("Left",		inRect.X);
		material.Value->SetScalarParameterValue("Right",	inRect.Y);
		material.Value->SetScalarParameterValue("Top",		inRect.Z);
		material.Value->SetScalarParameterValue("Bottom",	inRect.W);
	}
}
