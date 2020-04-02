#include "RoofMakers/ModernFlatRoof.h"
#include "BuildingsData.h"
#include "BuildingUtils.h"
#include "Basics.h"

#define FOUR_TIMES(something) something; something; something; something;


FBuildingMeshData UModernFlatRoofMaker::GenerateRoof_Implementation(FBuildingPart buildingPart, int firstSectionIndex,
                                                       UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial)
{
	FBuildingMeshData meshData;
	meshData.LastFreeIndex = firstSectionIndex;

	TArray<FVector>			Vertices;
	TArray<int>				Triangles;
	TArray<FVector>			Normals;
	TArray<FVector2D>		UV;
	TArray<FLinearColor>	VertexColors;


	for (auto& cont : buildingPart.OuterConts) {
		cont.FixClockwise();
	}
	for (auto& cont : buildingPart.InnerConts) {
		cont.FixClockwise(true);
	}
	
	int z = Vertices.Num();
	auto conts = TArray<FContour>(buildingPart.OuterConts);
	conts.Append(buildingPart.InnerConts);
	
	for (auto& cont : conts)
	{
		auto& contour = cont.Points;
		if (contour[0] == contour[contour.Num() - 1])
		{
			contour.RemoveAt(0);
		}
		float uvx = 0;
		
		z = Vertices.Num();
		
		for (int i = 0; i < contour.Num(); i++)
		{
			int iPrev = (i + contour.Num() - 1)	% contour.Num();
			int iNext = (i + 1)					% contour.Num();
			int iPlus = (i + 2)					% contour.Num();

			auto dirs		= MeshHelpers::GetNeighbourDirections(contour, i);
			auto dirsNext	= MeshHelpers::GetNeighbourDirections(contour, iNext);

			
			auto direction = FMath::Sign(FVector::CrossProduct(dirs.Key, dirs.Value).Z);
			auto delta	= (-dirs.Key.GetSafeNormal2D() + dirs.Value.GetSafeNormal2D()).GetSafeNormal2D() 
								* direction;

			auto directionNext = FMath::Sign(FVector::CrossProduct(dirsNext.Key, dirsNext.Value).Z);
			auto deltaNext	= (-dirsNext.Key.GetSafeNormal2D() + dirsNext.Value.GetSafeNormal2D()).GetSafeNormal2D() 
								* directionNext;

			auto v1 = contour[i];
			auto v2 = contour[i];
			auto v3 = contour[iNext];
			auto v4 = contour[iNext];

			auto v_1 = v1 + delta * BarrierWidth;
			auto v_2 = v2 + delta * BarrierWidth;
			auto v_3 = v3 + deltaNext * BarrierWidth;
			auto v_4 = v4 + deltaNext * BarrierWidth;
			
			v1.Z = buildingPart.Height;
			v2.Z = buildingPart.Height + BarrierHeight;
			v3.Z = buildingPart.Height;
			v4.Z = buildingPart.Height + BarrierHeight;

			v_1.Z = buildingPart.Height;
			v_2.Z = buildingPart.Height + BarrierHeight;
			v_3.Z = buildingPart.Height;
			v_4.Z = buildingPart.Height + BarrierHeight;

			Vertices.Add(v1);
			Vertices.Add(v2);
			Vertices.Add(v3);
			Vertices.Add(v4);

			Vertices.Add(v_1);
			Vertices.Add(v_2);
			Vertices.Add(v_3);
			Vertices.Add(v_4);

			Vertices.Add(v2);
			Vertices.Add(v4);
			Vertices.Add(v_2);
			Vertices.Add(v_4);

			FOUR_TIMES(
				UV.Add(FVector2D(uvx, 0));
				UV.Add(FVector2D(uvx, buildingPart.Height / 3));
				uvx += (v1 - v3).Size2D() / 10000;
			);

			UV.Add(FVector2D(v2.X	/ 100, v2.Y	/ 100));
			UV.Add(FVector2D(v4.X	/ 100, v4.Y	/ 100));
			UV.Add(FVector2D(v_2.X	/ 100, v_2.Y	/ 100));
			UV.Add(FVector2D(v_4.X	/ 100, v_4.Y	/ 100));


			FOUR_TIMES(Normals.Add( FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));
			FOUR_TIMES(Normals.Add(-FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));
			FOUR_TIMES(Normals.Add(FVector::UpVector));

			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));

			auto	vertexIndex = z + i * 12;

			Triangles.Add(vertexIndex + 0);
			Triangles.Add(vertexIndex + 1);
			Triangles.Add(vertexIndex + 3);
			Triangles.Add(vertexIndex + 0);
			Triangles.Add(vertexIndex + 3);
			Triangles.Add(vertexIndex + 2);

					vertexIndex += 4;

			Triangles.Add(vertexIndex + 0);
			Triangles.Add(vertexIndex + 3);
			Triangles.Add(vertexIndex + 1);
			Triangles.Add(vertexIndex + 0);
			Triangles.Add(vertexIndex + 2);
			Triangles.Add(vertexIndex + 3);

					vertexIndex += 4;

			Triangles.Add(vertexIndex + 1);
			Triangles.Add(vertexIndex + 0);
			Triangles.Add(vertexIndex + 2);
			Triangles.Add(vertexIndex + 1);
			Triangles.Add(vertexIndex + 2);
			Triangles.Add(vertexIndex + 3);
		}
	}

	meshData.Segments.Add(FMeshSegmentData{
		meshData.LastFreeIndex,
		Vertices,
		Triangles,
		Normals,
		UV,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		VertexColors,
		wallMaterial
	});

	meshData.LastFreeIndex++;

	return meshData;
}
