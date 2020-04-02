#include "RoofMakers/SkeletonSlopeRoof.h"
#include "BuildingsData.h"
#include "BuildingUtils.h"
#include "Basics.h"

#define FOUR_TIMES(something) something; something; something; something;

FBuildingMeshData USlopeRoofMaker::GenerateRoof_Implementation(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial,
	UMaterialInterface* roofMaterial)
{
		std::vector<FVector> nodes;
	std::vector<int> triangles;
	int contInd;

	TArray<FContour> outer, inner;

	for (auto& cont : buildingPart.OuterConts)
	{

		cont.FixClockwise();
		if (cont.Points[0] == cont.Points[cont.Points.Num() - 1])
		{
			cont.Points.RemoveAt(0);
		}
		auto cont1 = cont;
		auto& contour = cont.Points;
		for (int i = 0; i < contour.Num(); i++)
		{
			int iprev = (i + contour.Num() - 1) % contour.Num();
			int inext = (i + 1) % contour.Num();
			auto delta1 = ((contour[iprev] - contour[i]).GetSafeNormal2D() + (contour[inext] - contour[i]).GetSafeNormal2D()).GetSafeNormal2D() * FMath::Sign(FVector::CrossProduct(contour[i] - contour[iprev], contour[inext] - contour[i]).Z);
			cont1.Points[i] -= delta1 * 60;
		}
		cont1 = cont1.RemoveCollinear(0.01f);
		outer.Add(cont1);
	}

	for (auto& cont : buildingPart.InnerConts)
	{

		cont.FixClockwise(true);
		if (cont.Points[0] == cont.Points[cont.Points.Num() - 1])
		{
			cont.Points.RemoveAt(0);
		}
		auto cont1 = cont;
		auto& contour = cont.Points;

		for (int i = 0; i < contour.Num(); i++)
		{
			int iprev = (i + contour.Num() - 1) % contour.Num();
			int inext = (i + 1) % contour.Num();
			auto delta1 = ((contour[iprev] - contour[i]).GetSafeNormal2D() + (contour[inext] - contour[i]).GetSafeNormal2D()).GetSafeNormal2D() * FMath::Sign(FVector::CrossProduct(contour[i] - contour[iprev], contour[inext] - contour[i]).Z);
			cont1.Points[i] -= delta1 * 60;
		}
		cont1 = cont1.RemoveCollinear(0.01f);
		inner.Add(cont1);
	}

	Triangulate(outer, inner, nodes, triangles, "YYS0", buildingPart.RoofData, contInd);
	int contId2 = contInd;
	for (auto d : buildingPart.RoofData)
	{
		contId2 += d.Points.Num();
	}

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;

	for (int i = 0; i < triangles.size(); i += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			if (triangles[i + j] < contInd || triangles[i + j] >= contId2) {
				auto v = nodes[triangles[i + j]] + FVector::UpVector * (buildingPart.Height);
				Vertices.Add(v);
				UV.Add(FVector2D(0, 0));
				Triangles.Add(i + j);
			}
			else
			{
				auto v = nodes[triangles[i + j]] + FVector::UpVector * (buildingPart.Height + RoofHeight);
				Vertices.Add(v);
				UV.Add(FVector2D(0, 1));
				Triangles.Add(i + j);
			}

		}

		Normals.Add(FVector::CrossProduct(-(Vertices[i + 1] - Vertices[i]).GetSafeNormal(), Vertices[i + 2] - Vertices[i]).GetSafeNormal());
		Normals.Add(FVector::CrossProduct(-(Vertices[i + 1] - Vertices[i]).GetSafeNormal(), Vertices[i + 2] - Vertices[i]).GetSafeNormal());
		Normals.Add(FVector::CrossProduct(-(Vertices[i + 1] - Vertices[i]).GetSafeNormal(), Vertices[i + 2] - Vertices[i]).GetSafeNormal());
	}

	//Generating geometry
	FBuildingMeshData meshData;
	meshData.LastFreeIndex = firstSectionIndex;

	meshData.Segments.Add(FMeshSegmentData{
		meshData.LastFreeIndex,
		Vertices,
		Triangles,
		Normals,
		UV,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		TArray<FLinearColor>(),
		roofMaterial
		});

	return meshData;

}
