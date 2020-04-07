#include "RoofMakers/ModernFlatRoof.h"
#include "BuildingsData.h"
#include "BuildingUtils.h"
#include "Basics.h"

#define FOUR_TIMES(something) something; something; something; something;


FBuildingMeshData UModernFlatRoofMaker::GenerateRoof_Implementation(const FBuildingPart& inBuildingPart, int inFirstSectionIndex,
                                                       UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial)
{
	FBuildingMeshData meshData;
	meshData.LastFreeIndex = inFirstSectionIndex;

	TArray<FVector>			Vertices;
	TArray<int>				Triangles;
	TArray<FVector>			Normals;
	TArray<FVector2D>		UV;
	TArray<FLinearColor>	VertexColors;


	/*for (auto& cont : inBuildingPart.OuterConts) {
		cont.FixClockwise();
	}
	for (auto& cont : inBuildingPart.InnerConts) {
		cont.FixClockwise(true);
	}*/
	
	int vertCount = Vertices.Num();
	auto conts = TArray<FContour>(inBuildingPart.OuterConts);
	conts.Append(inBuildingPart.InnerConts);
	
	for (auto& cont : conts)
	{
		auto& contour = cont.Points;
		if (contour[0] == contour[contour.Num() - 1])
		{
			contour.RemoveAt(contour.Num() - 1);
		}
		float uvx = 0;
		
		vertCount = Vertices.Num();
		
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
			
			v1.Z = inBuildingPart.Height;
			v2.Z = inBuildingPart.Height + BarrierHeight;
			v3.Z = inBuildingPart.Height;
			v4.Z = inBuildingPart.Height + BarrierHeight;

			v_1.Z = inBuildingPart.Height;
			v_2.Z = inBuildingPart.Height + BarrierHeight;
			v_3.Z = inBuildingPart.Height;
			v_4.Z = inBuildingPart.Height + BarrierHeight;

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
				UV.Add(FVector2D(uvx, inBuildingPart.Height / 3));
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

			auto	vertexIndex = vertCount + i * 12;

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
		inWallMaterial
	});

	meshData.LastFreeIndex++;

	return meshData;
}
