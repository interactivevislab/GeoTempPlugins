#include "BuildingUtils.h"
#include <unordered_set>
#include "Basics.h"
#pragma warning( disable : 4456 )

#define FOUR_TIMES(something) something; something; something; something;


//FBuildingMeshData IRoofMaker::GenerateRoof_Implementation(FBuildingPart buildingPart, int firstSectionIndex,
//                                                          UMaterialInterface* wallMaterial,
//                                                          UMaterialInterface* roofMaterial)
//{
//	return FBuildingMeshData();
//}

FBuildingMeshData MeshHelpers::CalculateMeshData(FBuildingPart* buildingPart, int firstSectionIndex,
                                                 UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial, FString flags)
{
	FBuildingMeshData meshData;
	meshData.LastFreeIndex = firstSectionIndex;

	std::vector<FVector> nodes;
	std::vector<int> triangles;	

	if (!buildingPart->OverrideHeight && buildingPart->Height == 0)
	{
		buildingPart->Height = buildingPart->Floors * 350.0f + 50;
		buildingPart->MinHeight = buildingPart->MinFloors * 350.0f - 50;
		if (buildingPart->MinFloors == 0) buildingPart->MinHeight = -1500;
	}		

	Triangulate(buildingPart->OuterConts, buildingPart->InnerConts, nodes, triangles, std::string(TCHAR_TO_UTF8(*flags)));

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<FVector2D> UV1;
	TArray<FLinearColor> VertexColors;

	bool isInner = false;
	for (auto conts : { TArray<FContour>(buildingPart->OuterConts), TArray<FContour>(buildingPart->InnerConts) })
	{
		for (auto& cont : conts) {
			cont = cont.RemoveCollinear(); 
			if (isInner) cont.FixClockwise(true);
			auto& contour = cont.Points;

			int z = Vertices.Num();
			for (int i = 0; i < contour.Num(); i++)
			{
				int iminus = (i - 1 + contour.Num()) % contour.Num();
				int iplus = (i + 1) % contour.Num();
				int iplusplus = (i + 2) % contour.Num();
				int inext = iplus;
				auto v1 = contour[i];
				auto v2 = contour[i];
				auto v3 = contour[inext];
				auto v4 = contour[inext];
				v1.Z = buildingPart->MinHeight;
				v2.Z = buildingPart->Height;
				v3.Z = buildingPart->MinHeight;
				v4.Z = buildingPart->Height;
				if ((v1.GetAbsMax() > 10000000) || (v3.GetAbsMax() > 10000000))
				{
					continue;
				}

				Vertices.Add(v1);
				Vertices.Add(v2);
				Vertices.Add(v3);
				Vertices.Add(v4);

				auto delta = (v1 - v3).Size2D() / 300;
				auto deltaN = FMath::FloorToFloat(delta);
				auto offset = (delta - deltaN) / 2;
				UV.Add(FVector2D(-offset, (buildingPart->Height - buildingPart->MinHeight) / buildingPart->FloorHeight));
				UV.Add(FVector2D(-offset, 0));
				UV.Add(FVector2D(-offset + delta, (buildingPart->Height - buildingPart->MinHeight) / buildingPart->FloorHeight));
				UV.Add(FVector2D(-offset + delta, 0));

				FOUR_TIMES(UV1.Add(FVector2D(offset, deltaN)));
				FOUR_TIMES(VertexColors.Add(FLinearColor::Gray));

				auto baseNormal = FVector::CrossProduct(v3 - v1, FVector::UpVector).GetSafeNormal();

				FOUR_TIMES(Normals.Add(baseNormal));
				

				Triangles.Add(z + i * 4 + 0);
				Triangles.Add(z + i * 4 + 1);
				Triangles.Add(z + i * 4 + 3);
				Triangles.Add(z + i * 4 + 0);
				Triangles.Add(z + i * 4 + 3);
				Triangles.Add(z + i * 4 + 2);
			}
		}
		isInner = true;
	}

	meshData.Segments.Add(FMeshSegmentData {
		meshData.LastFreeIndex,
		Vertices,
		Triangles,
		Normals,
		UV,
		UV1,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		VertexColors,
		wallMaterial
	});

	meshData.LastFreeIndex++;

	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UV.Empty();
	VertexColors.Empty();

	int z = Vertices.Num();
	for (auto& node : nodes)
	{
		auto v = node;
		v.Z = buildingPart->MinHeight;
		Vertices.Add(v);
		Normals.Add(FVector::DownVector);
		UV.Add(FVector2D::ZeroVector);
		VertexColors.Add(FLinearColor::Gray);
	}
	for (int i = 0; i < triangles.size(); i += 3)
	{
		Triangles.Add(z + triangles[i]);
		Triangles.Add(z + triangles[i + 2]);
		Triangles.Add(z + triangles[i + 1]);
	}
	z = Vertices.Num();
	for (auto& node : nodes)
	{
		auto v = node;
		v.Z = buildingPart->Height;
		Vertices.Add(v);
		Normals.Add(FVector::UpVector);
		UV.Add(FVector2D(v.X / 100, v.Y / 100));
		VertexColors.Add(FLinearColor::Gray);
	}

	for (auto ind : triangles)
	{
		Triangles.Add(z + ind);
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
		roofMaterial
	});

	meshData.LastFreeIndex++;

	if (RoofMakers.Contains(buildingPart->Owner->RoofType))
	{
		//auto val = RoofMakers.FindRef(buildingPart->Owner->RoofType);
		//val->GenerateRoof(*buildingPart, meshData.LastFreeIndex, wallMaterial, roofMaterial);
	} else if (RoofMakers.Contains("Default"))
	{
		//auto val = RoofMakers.Find("Default");
		//(*val)->GenerateRoof(*buildingPart, meshData.LastFreeIndex, wallMaterial, roofMaterial);	
	}
	
	return meshData;
}



void MeshHelpers::ConstructProceduralMesh(UProceduralMeshComponent* procMesh, FBuildingMeshData meshData)
{
	for (auto& segmentData : meshData.Segments) {
		procMesh->CreateMeshSection_LinearColor(
			segmentData.SectionIndex, 
			segmentData.Vertices,
			segmentData.Triangles,
			segmentData.Normals,
			segmentData.UV0,
			segmentData.UV1,
			segmentData.UV2,
			segmentData.UV3,
			segmentData.VertexColors,
			TArray<FProcMeshTangent>(), 
			true
		);
		procMesh->SetMaterial(segmentData.SectionIndex, segmentData.Material);
	}
}


void MeshHelpers::ConstructRuntimeMesh(URuntimeMeshComponent* runtimeMesh, FBuildingMeshData meshData)
{
	for (auto& segmentData : meshData.Segments)
	{
		TArray<FColor> vertexColors;
		for (auto& linearColor : segmentData.VertexColors) {
			vertexColors.Add(linearColor.ToFColor(true));
		}

		runtimeMesh->CreateMeshSection(
			segmentData.SectionIndex,
			segmentData.Vertices,
			segmentData.Triangles,
			segmentData.Normals,
			segmentData.UV0,
			segmentData.UV1,
			vertexColors,
			TArray<FRuntimeMeshTangent>(),
			true
		);
		runtimeMesh->SetSectionMaterial(segmentData.SectionIndex, segmentData.Material);
	}
}

