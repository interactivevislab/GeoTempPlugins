#include "BuildingUtils.h"
#include <unordered_set>
#include "Basics.h"
#pragma warning( disable : 4456 )

#define FOUR_TIMES(something) something; something; something; something;

TMap<FString, TScriptInterface<IRoofMaker>> MeshHelpers::RoofMakers = TMap<FString, TScriptInterface<IRoofMaker>>();

const float FLOOR_HEIGHT = 350;
FBuildingMeshData MeshHelpers::CalculateMeshData(const FBuilding& inBuilding, const FBuildingPart& inBuildingPart,
                                                 UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial, const FString& inTriangulationFlags)
{
	FBuildingMeshData meshData;

	TArray<FVector> nodes;
	TArray<int> triangles;	

	Triangulate(inBuildingPart.OuterConts, inBuildingPart.InnerConts, nodes, triangles, inTriangulationFlags);

	TArray<FVector>			Vertices;
	TArray<int>				Triangles;
	TArray<FVector>			Normals;
	TArray<FVector2D>		UV;
	TArray<FVector2D>		UV1;
	TArray<FLinearColor>	VertexColors;

	bool isInner = false;
	for (auto conts : { TArray<FContour>(inBuildingPart.OuterConts), TArray<FContour>(inBuildingPart.InnerConts) })
	{
		for (auto& cont : conts) 
		{
			cont = cont.RemoveCollinear(); 
			if (isInner) 
			{
				cont.FixClockwise(true);
			}
			
			auto& contour = cont.Points;

			int z = Vertices.Num();
			for (int i = 0; i < contour.Num(); i++)
			{
				int iplus = (i + 1) % contour.Num();
				int inext = iplus;
				
				auto v1 = contour[i];
				auto v2 = contour[i];
				auto v3 = contour[inext];
				auto v4 = contour[inext];
				
				v1.Z = inBuildingPart.MinHeight;
				v2.Z = inBuildingPart.Height;
				v3.Z = inBuildingPart.MinHeight;
				v4.Z = inBuildingPart.Height;
				
				if ((v1.GetAbsMax() > 10000000) || (v3.GetAbsMax() > 10000000))
				{
					continue;
				}

				Vertices.Add(v1);
				Vertices.Add(v2);
				Vertices.Add(v3);
				Vertices.Add(v4);

				auto delta	= (v1 - v3).Size2D() / 300;
				auto deltaN	= FMath::FloorToFloat(delta);
				auto offset	= (delta - deltaN) / 2;
				auto height = (inBuildingPart.Height - inBuildingPart.MinHeight) ;
				UV.Add(FVector2D(-offset,			height / inBuildingPart.FloorHeight));
				UV.Add(FVector2D(-offset,			0));
				UV.Add(FVector2D(-offset + delta,	height / inBuildingPart.FloorHeight));
				UV.Add(FVector2D(-offset + delta,	0));

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

	meshData.Sections.Add(FMeshSectionData {
		Vertices,
		Triangles,
		Normals,
		UV,
		UV1,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		VertexColors,
		inWallMaterial
	});

	Vertices		.Empty();
	Triangles		.Empty();
	Normals			.Empty();
	UV				.Empty();
	VertexColors	.Empty();

	int z = Vertices.Num();
	for (auto& node : nodes)
	{
		auto v = node;
		v.Z = inBuildingPart.MinHeight;
		
		Vertices.		Add(v							);
		Normals.		Add(FVector::DownVector		);
		UV.				Add(FVector2D::ZeroVector	);
		VertexColors.	Add(FLinearColor::Gray		);
	}
	for (int i = 0; i < triangles.Num(); i += 3)
	{
		Triangles.Add(z + triangles[i]		);
		Triangles.Add(z + triangles[i + 2]	);
		Triangles.Add(z + triangles[i + 1])	;
	}
	z = Vertices.Num();
	for (auto& node : nodes)
	{
		auto v = node;
		v.Z = inBuildingPart.Height;
		Vertices		.Add(v);
		Normals			.Add(FVector::UpVector);
		UV				.Add(FVector2D(v.X / 100, v.Y / 100));
		VertexColors	.Add(FLinearColor::Gray);
	}

	for (auto ind : triangles)
	{
		Triangles.Add(z + ind);
	}

	meshData.Sections.Add(FMeshSectionData{
		Vertices,
		Triangles,
		Normals,
		UV,
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		TArray<FVector2D>(),
		VertexColors,
		inRoofMaterial
	});
	auto RoofType = inBuildingPart.RoofType.IsEmpty() ? inBuilding.RoofType : inBuildingPart.RoofType;
	if (RoofMakers.Contains(RoofType))
	{
		auto val = RoofMakers.FindRef(RoofType);
		val->GenerateRoof(inBuildingPart, inWallMaterial, inRoofMaterial);
	}
	else if (RoofMakers.Contains("Default"))
	{
		auto val = RoofMakers.FindRef("Default");
		val->GenerateRoof(inBuildingPart, inWallMaterial, inRoofMaterial);	
	}
	
	return meshData;
}



void MeshHelpers::ConstructProceduralMesh(UProceduralMeshComponent* inProcMesh, const FBuildingMeshData& inMeshData, int inFirstSectionIndex)
{	
	for (auto& segmentData : inMeshData.Sections) 
	{
		inProcMesh->CreateMeshSection_LinearColor(
			inFirstSectionIndex, 
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
		inProcMesh->SetMaterial(inFirstSectionIndex, segmentData.Material);
		inFirstSectionIndex++;
	}
}


void MeshHelpers::ConstructRuntimeMesh(URuntimeMeshComponent* runtimeMesh, const FBuildingMeshData& inMeshData, int inFirstSectionIndex)
{
	for (auto& segmentData : inMeshData.Sections)
	{
		TArray<FColor> vertexColors;
		for (auto& linearColor : segmentData.VertexColors) 
		{
			vertexColors.Add(linearColor.ToFColor(true));
		}
		if(segmentData.Triangles.Num() == 0 || segmentData.Vertices.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Attempt to create an empty mesh"));
			return;
		}
		runtimeMesh->CreateMeshSection(
			inFirstSectionIndex,
			segmentData.Vertices,
			segmentData.Triangles,
			segmentData.Normals,
			segmentData.UV0,
			segmentData.UV1,
			vertexColors,
			TArray<FRuntimeMeshTangent>(),
			true
		);
		runtimeMesh->SetSectionMaterial(inFirstSectionIndex, segmentData.Material);
		inFirstSectionIndex++;
	}
}

