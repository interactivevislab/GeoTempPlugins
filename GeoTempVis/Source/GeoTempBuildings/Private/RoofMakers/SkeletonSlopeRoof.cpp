#include "RoofMakers/SkeletonSlopeRoof.h"
#include "BuildingsData.h"
#include "BuildingUtils.h"
#include "Basics.h"

#define FOUR_TIMES(something) something; something; something; something;



FBuildingMeshData USlopeRoofMaker::GenerateRoof_Implementation(FBuildingPart inBuildingPart, int inFirstSectionIndex, UMaterialInterface* inWallMaterial,
	UMaterialInterface* inRoofMaterial)
{
	TArray<FVector> nodes;
	TArray<int> triangles;
	int contInd;

	TArray<FContour> outer, inner;

	for (auto& cont : inBuildingPart.OuterConts)
	{
		cont.FixClockwise();
		
		if (cont.Points[0] == cont.Points[cont.Points.Num() - 1])
		{
			cont.Points.RemoveAt(cont.Points.Num() - 1);
		}

		auto& contour = cont.Points;
		
		auto cont1 = cont;
		for (int i = 0; i < contour.Num(); i++)
		{
			auto dirs = MeshHelpers::GetNeighbourDirections(contour, i);
			
			auto direction = FMath::Sign(FVector::CrossProduct(dirs.Key, dirs.Value).Z);
			auto delta1	= (-dirs.Key.GetSafeNormal2D() + dirs.Value.GetSafeNormal2D()).GetSafeNormal2D() * direction;
			cont1.Points[i] -= delta1 * 60;
		}
		cont1 = cont1.RemoveCollinear(0.01f);
		outer.Add(cont1);
	}

	for (auto& cont : inBuildingPart.InnerConts)
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
			auto dirs = MeshHelpers::GetNeighbourDirections(contour, i);
			auto direction = FMath::Sign(FVector::CrossProduct(dirs.Key, dirs.Value).Z);
			auto delta1 = (-dirs.Key.GetSafeNormal2D() + dirs.Value.GetSafeNormal2D()).GetSafeNormal2D() * direction;
			cont1.Points[i] -= delta1 * 60;
		}
		cont1 = cont1.RemoveCollinear(0.01f);
		inner.Add(cont1);
	}

	Triangulate(outer, inner, nodes, triangles, "YYS0", inBuildingPart.RoofData, contInd);
	int contId2 = contInd;
	for (auto d : inBuildingPart.RoofData)
	{
		contId2 += d.Points.Num();
	}

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;

	for (int i = 0; i < triangles.Num(); i += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			if (triangles[i + j] < contInd || triangles[i + j] >= contId2) 
			{
				auto v = nodes[triangles[i + j]] + FVector::UpVector * (inBuildingPart.Height);
				Vertices.Add(v);
				UV.Add(FVector2D(0, 0));
				Triangles.Add(i + j);
			}
			else
			{
				auto v = nodes[triangles[i + j]] + FVector::UpVector * (inBuildingPart.Height + RoofHeight);
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
	meshData.LastFreeIndex = inFirstSectionIndex;

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
		inRoofMaterial
		});

	return meshData;

}
