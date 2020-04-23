#include "RoadBuilder.h"

#include "RoadNetworkActor.h"


#define LIST_4_TIMES(something) something, something, something, something
#define LIST_8_TIMES(something) LIST_4_TIMES(something), LIST_4_TIMES(something)


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

void URoadBuilder::RemoveRoadNetworkActor()
{
	if (roadNetworkActor)
	{
		roadNetworkActor->Destroy();
		roadNetworkActor = nullptr;
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
