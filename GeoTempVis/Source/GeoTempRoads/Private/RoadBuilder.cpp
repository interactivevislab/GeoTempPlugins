#include "RoadBuilder.h"

#include "RuntimeMeshActor.h"


#define LIST_4_TIMES(something) something, something, something, something
#define LIST_8_TIMES(something) LIST_4_TIMES(something), LIST_4_TIMES(something)


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
			auto endPoint	= segment.AllPoints[i + 1];
			startPoint.Z	= roadZ;
			endPoint.Z		= roadZ - 1;

			auto pointDelta = FVector::CrossProduct((startPoint - endPoint).GetSafeNormal(), FVector(0, 0, 1));
			auto curtainsDelta1 =	 pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			auto curtainsDelta2 =	-pointDelta * inCurtainsWidth - FVector(0, 0, inRoadHeight);
			
			pointDelta *= (segment.Width * 50);
			
			auto point0 = startPoint	+ pointDelta;
			auto point1 = startPoint	- pointDelta;
			auto point2 = endPoint		+ pointDelta;
			auto point3 = endPoint		- pointDelta;

			auto indicesDelta = sectionData.Vertices.Num();
			sectionData.Vertices.Append({
				point0,
				point1,
				point2,
				point3,
			});

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
				point0 + curtainsDelta1,
				point0,
				point2 + curtainsDelta1,
				point2,

				point1,
				point1 + curtainsDelta2,
				point3,
				point3 + curtainsDelta2
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
			sectionData.Uv1					.Append({ LIST_4_TIMES(uv1Data) });
			sectionData.Normals			.Append({ LIST_4_TIMES(FVector::UpVector) });
			sectionData.VertexColors		.Append({ LIST_4_TIMES(FColor(1, 1, 1, 1)) });
			sectionData.Tangents			.Append({ LIST_4_TIMES(FRuntimeMeshTangent()) });
			
			outCurtainsMeshData.Uv0			.Append({ uv00, uv01c, uv02, uv03c, uv00, uv01c, uv02, uv03c });
			outCurtainsMeshData.Uv1			.Append({ LIST_8_TIMES(uv1Data) });			
			outCurtainsMeshData.Normals		.Append({ LIST_4_TIMES(curtainsNormal1), LIST_4_TIMES(curtainsNormal2) });			
			outCurtainsMeshData.VertexColors.Append({ LIST_8_TIMES(FColor(1, 1, 1, 1)) });			
			outCurtainsMeshData.Tangents	.Append({ LIST_8_TIMES(FRuntimeMeshTangent()) });

			for (auto point : { startPoint, endPoint })
			{
				bool isReversedCup = (point == endPoint);
				const int capDensity = 8;
				auto yDelta = FVector::CrossProduct(pointDelta, FVector::UpVector);
				point -= FVector(0, 0, 2);
				
				indicesDelta				= sectionData.Vertices.Num();
				auto indicesDelta_curtains	= outCurtainsMeshData.Vertices.Num();
				
				sectionData.Vertices	.Add(point);
				sectionData.Uv0			.Add(FVector2D(0.5 * segment.Lanes, 0.5));
				sectionData.Uv1			.Add(uv1Data);
				sectionData.Normals		.Add(FVector::UpVector);
				sectionData.VertexColors.Add(FColor::White);
				sectionData.Tangents	.Add(FRuntimeMeshTangent());

				FVector radiusDeltas	[capDensity + 1];
				FVector2D uvs			[capDensity + 1];
				FVector2D uvs_curtains	[capDensity + 1];

				for (int j = 0; j <= capDensity; j++)
				{
					float angle = PI / capDensity * j;

					float x = FMath::Cos(angle);
					float y = FMath::Sin(angle);

					radiusDeltas[j]	= (isReversedCup ? 1 : -1) * (pointDelta * x + yDelta * y);
					uvs[j]			= FVector2D(0, static_cast<float>(j) / capDensity * segment.Lanes);
					uvs_curtains[j]	= FVector2D(x, y);
				}

				for (int j = 0; j <= capDensity; j++)
				{
					auto radiusDelta	= radiusDeltas[j];
					auto size			= radiusDelta.Size();
					auto curtainsDelta	= radiusDelta / size * inCurtainsWidth - FVector(0, 0, inRoadHeight);
					auto curtainNormal	= FVector::CrossProduct(
						(radiusDeltas[FMath::Max(j - 1, 0)] - radiusDelta).GetSafeNormal(), 
						(-curtainsDelta).GetSafeNormal());
					
					
					sectionData.Vertices	.Add(point + radiusDelta);
					sectionData.Uv0			.Add(uvs[j]);
					sectionData.Uv1			.Add(uv1Data);
					sectionData.Normals		.Add(FVector::UpVector);
					sectionData.VertexColors.Add(FColor::White);
					sectionData.Tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.Vertices	.Add(point + radiusDelta);
					outCurtainsMeshData.Uv0			.Add(uvs_curtains[j]);
					outCurtainsMeshData.Uv1			.Add(uv1Data);
					outCurtainsMeshData.Normals		.Add(curtainNormal);
					outCurtainsMeshData.VertexColors.Add(FColor::White);
					outCurtainsMeshData.Tangents	.Add(FRuntimeMeshTangent());

					outCurtainsMeshData.Vertices	.Add(point + radiusDelta + curtainsDelta);
					outCurtainsMeshData.Uv0			.Add(2 * uvs_curtains[j]);
					outCurtainsMeshData.Uv1			.Add(uv1Data);
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

	inRuntimeMesh->CreateMeshSection(inSectionIndex, sectionData.Vertices, sectionData.Indices, sectionData.Normals,
		sectionData.Uv0, sectionData.Uv1, sectionData.VertexColors, sectionData.Tangents, false);
	inRuntimeMesh->SetMaterial(inSectionIndex, inMaterial);
}


void URoadBuilder::SpawnRoadNetworkActor(FRoadNetwork inRoadNetwork)
{
	roadMaterials = {
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

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Name = "RoadNetworkActor";
	auto roadNetworkActor = GetWorld()->SpawnActor<ARuntimeMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	auto runtimeMesh = roadNetworkActor->GetRuntimeMeshComponent();

	MeshSectionData curtainsMeshData;
	ConstructRoadMeshSection(runtimeMesh, autoSegments, AUTO_MATERIAL_INDEX, 
		roadMaterials[AUTO_MATERIAL_INDEX], curtainsMeshData);
	ConstructRoadMeshSection(runtimeMesh, railSegments, RAIL_MATERIAL_INDEX, 
		roadMaterials[RAIL_MATERIAL_INDEX], curtainsMeshData);

	runtimeMesh->CreateMeshSection(CURTAINS_MATERIAL_INDEX, curtainsMeshData.Vertices, curtainsMeshData.Indices, curtainsMeshData.Normals,
		curtainsMeshData.Uv0, curtainsMeshData.Uv1, curtainsMeshData.VertexColors, curtainsMeshData.Tangents, false);
	runtimeMesh->SetMaterial(CURTAINS_MATERIAL_INDEX, roadMaterials[CURTAINS_MATERIAL_INDEX]);
}
