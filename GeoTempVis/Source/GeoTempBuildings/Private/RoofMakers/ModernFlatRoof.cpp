#include "RoofMakers/ModernFlatRoof.h"
#include "BuildingsData.h"
#include "BuildingUtils.h"
#include "Basics.h"

#define FOUR_TIMES(something) something; something; something; something;


FBuildingMeshData UModernFlatRoofMaker::GenerateRoof_Implementation(const FBuildingPart& inBuildingPart, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial)
{
	FBuildingMeshData meshData;

	TArray<FVector>			vertices;
	TArray<int>				triangles;
	TArray<FVector>			normals;
	TArray<FVector2D>		UV;
	TArray<FLinearColor>	vertexColors;


	/*for (auto& cont : inBuildingPart.OuterConts) {
		cont.FixClockwise();
	}
	for (auto& cont : inBuildingPart.InnerConts) {
		cont.FixClockwise(true);
	}*/
	
	int vertCount = vertices.Num();
	auto conts = TArray<FContour>(inBuildingPart.OuterConts);
	conts.Append(inBuildingPart.InnerConts);
	
	for (auto& cont : conts)
	{		
		float uvx = 0;
		
		vertCount = vertices.Num();
		int num = cont.Points.Num();
		for (int i = 0; i < num; i++)
		{			
			int iNext = (i + 1)	% num;

			auto dirs		= MeshHelpers::GetNeighbourDirections(cont.Points, i);
			auto dirsNext	= MeshHelpers::GetNeighbourDirections(cont.Points, iNext);

			
			auto direction = FMath::Sign(FVector::CrossProduct(dirs.Key, dirs.Value).Z);
			auto delta	= (-dirs.Key.GetSafeNormal2D() + dirs.Value.GetSafeNormal2D()).GetSafeNormal2D() 
								* direction;

			auto directionNext = FMath::Sign(FVector::CrossProduct(dirsNext.Key, dirsNext.Value).Z);
			auto deltaNext	= (-dirsNext.Key.GetSafeNormal2D() + dirsNext.Value.GetSafeNormal2D()).GetSafeNormal2D() 
								* directionNext;

			auto v1 = cont.Points[i];
			auto v2 = cont.Points[i];
			auto v3 = cont.Points[iNext];
			auto v4 = cont.Points[iNext];

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

			vertices.Add(v1);
			vertices.Add(v2);
			vertices.Add(v3);
			vertices.Add(v4);

			vertices.Add(v_1);
			vertices.Add(v_2);
			vertices.Add(v_3);
			vertices.Add(v_4);

			vertices.Add(v2);
			vertices.Add(v4);
			vertices.Add(v_2);
			vertices.Add(v_4);

			FOUR_TIMES(
				UV.Add(FVector2D(uvx, 0));
				UV.Add(FVector2D(uvx, inBuildingPart.Height / 3));
				uvx += (v1 - v3).Size2D() / 10000;
			);

			UV.Add(FVector2D(v2.X	/ 100, v2.Y	/ 100));
			UV.Add(FVector2D(v4.X	/ 100, v4.Y	/ 100));
			UV.Add(FVector2D(v_2.X	/ 100, v_2.Y	/ 100));
			UV.Add(FVector2D(v_4.X	/ 100, v_4.Y	/ 100));


			FOUR_TIMES(normals.Add( FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));
			FOUR_TIMES(normals.Add(-FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal()));
			FOUR_TIMES(normals.Add(FVector::UpVector));

			FOUR_TIMES(vertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(vertexColors.Add(FLinearColor::Gray));
			FOUR_TIMES(vertexColors.Add(FLinearColor::Gray));

			auto	vertexIndex = vertCount + i * 12;

			triangles.Add(vertexIndex + 0);
			triangles.Add(vertexIndex + 1);
			triangles.Add(vertexIndex + 3);
			triangles.Add(vertexIndex + 0);
			triangles.Add(vertexIndex + 3);
			triangles.Add(vertexIndex + 2);

					vertexIndex += 4;

			triangles.Add(vertexIndex + 0);
			triangles.Add(vertexIndex + 3);
			triangles.Add(vertexIndex + 1);
			triangles.Add(vertexIndex + 0);
			triangles.Add(vertexIndex + 2);
			triangles.Add(vertexIndex + 3);

					vertexIndex += 4;

			triangles.Add(vertexIndex + 1);
			triangles.Add(vertexIndex + 0);
			triangles.Add(vertexIndex + 2);
			triangles.Add(vertexIndex + 1);
			triangles.Add(vertexIndex + 2);
			triangles.Add(vertexIndex + 3);
		}
	}

	meshData.Sections.Add(FMeshSectionData{
		vertices,
		triangles,
		normals,
		UV,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		vertexColors,
		inWallMaterial
	});

	return meshData;
}
