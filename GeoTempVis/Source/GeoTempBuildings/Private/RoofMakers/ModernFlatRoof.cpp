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

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<FLinearColor> VertexColors;

	//walls
	for (auto& cont : buildingPart.OuterConts) {
		cont.FixClockwise();
	}
	for (auto& cont : buildingPart.InnerConts) {
		cont.FixClockwise(true);
	}
	int z = Vertices.Num();
	auto conts = TArray<FContour>(buildingPart.OuterConts);
	conts.Append(buildingPart.InnerConts);
	int contNum = 0;
	for (auto& cont : conts)
	{
		auto& contour = cont.Points;
		if (contour[0] == contour[contour.Num() - 1]) contour.RemoveAt(0);
		float uvx = 0;
		//outer
		z = Vertices.Num();
		for (int i = 0; i < contour.Num(); i++)
		{
			int iprev = (i + contour.Num() - 1) % contour.Num();
			int inext = (i + 1) % contour.Num();
			int iplus = (i + 2) % contour.Num();
			auto delta1 = ((contour[iprev] - contour[i]).GetSafeNormal2D() + (contour[inext] - contour[i]).GetSafeNormal2D()).GetSafeNormal2D() 
				* FMath::Sign(FVector::CrossProduct(contour[i] - contour[iprev], contour[inext] - contour[i]).Z);
			auto delta2 = ((contour[i] - contour[inext]).GetSafeNormal2D() + (contour[iplus] - contour[inext]).GetSafeNormal2D()).GetSafeNormal2D() 
				* FMath::Sign(FVector::CrossProduct(contour[inext] - contour[i], contour[iplus] - contour[inext]).Z);

			auto v1 = contour[i];
			auto v2 = contour[i];
			auto v3 = contour[inext];
			auto v4 = contour[inext];

			auto v_1 = v1 + delta1 * 80;
			auto v_2 = v2 + delta1 * 80;
			auto v_3 = v3 + delta2 * 80;
			auto v_4 = v4 + delta2 * 80;
			v1.Z = buildingPart.Height;
			v2.Z = buildingPart.Height + 70;
			v3.Z = buildingPart.Height;
			v4.Z = buildingPart.Height + 70;

			v_1.Z = buildingPart.Height;
			v_2.Z = buildingPart.Height + 70;
			v_3.Z = buildingPart.Height;
			v_4.Z = buildingPart.Height + 70;

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

			UV.Add(FVector2D(v2.X / 100, v2.Y / 100));
			UV.Add(FVector2D(v4.X / 100, v4.Y / 100));
			UV.Add(FVector2D(v_2.X / 100, v_2.Y / 100));
			UV.Add(FVector2D(v_4.X / 100, v_4.Y / 100));


			FOUR_TIMES(Normals.Add(FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));
			FOUR_TIMES(Normals.Add(-FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));

			FOUR_TIMES(Normals.Add(FVector::UpVector));

			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));

			auto vertexIndex = z + i * 12;

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
		contNum++;
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
