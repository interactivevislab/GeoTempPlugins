#include "WaterBuilder.h"


void UWaterBuilder::AddWaterToMeshSingleSection(URuntimeMeshComponent * meshComponent, const TArray<FMultipolygonData>& inPolygonData, UMaterialInterface * material)
{
	TArray<FVector> vertices;
	TArray<int> indices;
	TArray<FVector> normales;
	TArray<FVector2D> uv0;
	TArray<FColor> vertexColors;
	TArray<FRuntimeMeshTangent> tangents;

	int segmentIndex = 0;

	//LAKES
	TArray<FRuntimeMeshTangent> tangentsLakes;
	TArray<FColor> colors;
	for (auto lakeData : inPolygonData)
	{

		TArray<FVector> nodes;
		TArray<int> triangles;

		Triangulate(lakeData.Outer, lakeData.Holes, nodes, triangles);

		TArray<FVector> Vertices;
		TArray<int> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UV;
		TArray<FVector2D> UV1;
		TArray<FLinearColor> VertexColors;

		int z = vertices.Num();
		for (auto node : nodes)
		{
			auto v = node;
			v.Z = WaterZ;
			Vertices.Add(v);
			Normals.Add(FVector::UpVector);
			UV.Add(FVector2D(v.X / 100, v.Y / 100));
			VertexColors.Add(FLinearColor::Gray);
		}

		for (auto ind : triangles)
		{
			Triangles.Add(z + ind);
		}

		tangentsLakes.Init(FRuntimeMeshTangent(), Normals.Num());
		colors.Init(FColor(0.5, 0.5, 0.5), Normals.Num());

		tangents.Append(tangentsLakes);
		vertexColors.Append(colors);
		vertices.Append(Vertices);
		indices.Append(Triangles);
		normales.Append(Normals);
		uv0.Append(UV);
	}
	//LAKES

	auto sectionIndex = meshComponent->GetNumSections();
	meshComponent->CreateMeshSection(sectionIndex, vertices, indices, normales, uv0, vertexColors, tangents, true);
	meshComponent->SetMaterial(sectionIndex, material);
}

void UWaterBuilder::SpawnWaterActor(const TArray<FMultipolygonData>& inPolygonData)
{
	if (!WaterMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("No material assigned to WaterBuilder! Abort mesh building."));
		return;
	}
	UMaterialInstanceDynamic* waterMaterial = UMaterialInstanceDynamic::Create(WaterMaterial, this);

	if (waterActor)
	{
		RemoveWaterActor();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetOwner();
	SpawnInfo.Name = "WaterActor";
	waterActor = GetWorld()->SpawnActor<AWaterActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	waterActor->SetActorLabel(SpawnInfo.Name.ToString());
	waterActor->SetMobility(EComponentMobility::Movable);
	auto runtimeMesh = waterActor->GetRuntimeMeshComponent();

	AddWaterToMeshSingleSection(runtimeMesh, inPolygonData, waterMaterial);

	waterActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
}

void UWaterBuilder::RemoveWaterActor()
{
	if (waterActor)
	{
		waterActor->Destroy();
		waterActor = nullptr;
	}

	TArray<AWaterActor*> toDestroy;
	for (auto child : GetOwner()->Children)
	{
		auto castChild = Cast<AWaterActor>(child);
		if (castChild)
		{
			toDestroy.Add(castChild);
		}
	}
	for (auto child : toDestroy)
	{
		child->Destroy();
	}
}
