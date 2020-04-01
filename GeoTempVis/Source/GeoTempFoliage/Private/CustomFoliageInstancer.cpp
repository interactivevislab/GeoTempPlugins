// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomFoliageInstancer.h"
#include "CoreMinimal.h"
#include "Engine/Private/InstancedStaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#pragma warning( disable : 4456)
#pragma warning( disable : 4458)

ACustomFoliageInstancer::ACustomFoliageInstancer()
{
	PrimaryActorTick.bCanEverTick = true;

	Width				= 300;
	Height				= 300;
	CellSize.X			= 20;
	CellSize.Y			= 20;
	SpawnTranslate.X	= 0;
	SpawnTranslate.Y	= 0;
	SpawnAngle			= 0;
	MeshLayersOption	= ELayersOption::PolyLayered;

	ObjectQuery.Add(EObjectTypeQuery::ObjectTypeQuery1);

	Projection	= ProjectionType::WGS84;
	OriginLat	= 60.009521f;
	OriginLon	= 29.730619f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Roooot"));
	SetRootComponent(Root);
}


void ACustomFoliageInstancer::BeginPlay()
{
	Super::BeginPlay();

	for (FFoliageMeshInfo& meshInfo : FoliageMeshes)
	{
		for (int x = 0; x < meshInfo.Mesh->StaticMaterials.Num(); ++x)
		{
			UMaterialInterface* material = meshInfo.Mesh->GetMaterial(x);
			if (material->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				meshInfo.MaterialInstances.Add(x, Cast<UMaterialInstanceDynamic>(material));
			}
			else {
				auto materialName		= FName(*(*TEXT("NewInstancedMaterial") + "_" + FString::FromInt(x)));
				auto dynamicMaterial	= UMaterialInstanceDynamic::Create(material, meshInfo.Mesh, materialName);

				meshInfo.MaterialInstances.Add(x, dynamicMaterial);
			}
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerWidth",	Width		);
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerHeight",	Height		);
			meshInfo.MaterialInstances[x]->SetTextureParameterValue	("Start",			StartTarget	);
			meshInfo.MaterialInstances[x]->SetTextureParameterValue	("End",				EndTarget	);

			if (IsValid(TypesTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", TypesTarget);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", BlankMask);
			}
		}
	}
}


void ACustomFoliageInstancer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (FFoliageMeshInfo Info : FoliageMeshes)
	{
		InterpolateFoliageWithMaterial(Info);
	}
}


void ACustomFoliageInstancer::BufferMask(UTexture2D* inInitialMask)
{
	MaskBuffer.Empty();
	inInitialMask->UpdateResource();
	TArray<FColor> OutData;

	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			auto texture	= ((FTexture2DResource*)inInitialMask->Resource)->GetTexture2DRHI();
			auto rect		= FIntRect(0, 0, inInitialMask->Resource->GetSizeX(), inInitialMask->Resource->GetSizeY());
			RHICmdList.ReadSurfaceData(texture, rect, OutData, FReadSurfaceDataFlags());
		}
	);
	FlushRenderingCommands();
	for (int x = 0; x < inInitialMask->PlatformData->Mips[0].SizeY; x++)
	{
		for (int y = 0; y < inInitialMask->PlatformData->Mips[0].SizeX; y++)
		{
			MaskBuffer.Add((float)(OutData[x * inInitialMask->GetSizeX() + y].R) / 255);
		}
	}
}


void ACustomFoliageInstancer::InterpolateFoliageWithMaterial(FFoliageMeshInfo Fol)
{
	auto meshPtr = FoliageInstancers.FindRef(Fol.Mesh);

	if (!meshPtr || !IsValid(meshPtr)) {
		return;
	}
	meshPtr->SetScalarParameterValueOnMaterials("Interpolation", CurrentInterpolation);
}


void ACustomFoliageInstancer::FillFoliage_BP(FVector4 componentRect, bool DebugDraw, bool updateMaskBuffer)
{
	Width = Height = componentRect.Y - componentRect.X;

	const int StartCullDistance = 2000000000;
	const int EndCullDistance	= 2100000000;
	FVector componentOffset		= FVector(componentRect.X, componentRect.Z, 0);

	TArray<FFoliageMeshInfo> ArrayOfMeshInfos;
	TArray<UHierarchicalInstancedStaticMeshComponent*> ArrayOfInstancers;

	UpdateBuffer();

	for (FFoliageMeshInfo meshInfo : FoliageMeshes)
	{
		UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

		for (int x = 0; x < meshInfo.Mesh->StaticMaterials.Num(); ++x)
		{
			UMaterialInterface* material = meshInfo.Mesh->GetMaterial(x);

			if (material->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				meshInfo.MaterialInstances.Add(x, Cast<UMaterialInstanceDynamic>(material));
			}
			else {
				auto materialName	 = FName(*(*TEXT("NewInstancedMaterial") + "_" + FString::FromInt(x)));
				auto dynamicMaterial = UMaterialInstanceDynamic::Create(material, meshInfo.Mesh, materialName);

				meshInfo.MaterialInstances.Add(x, dynamicMaterial);
			}
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerWidth",	Width		);
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerHeight", Height		);
			meshInfo.MaterialInstances[x]->SetTextureParameterValue	("Start",			StartTarget	);
			meshInfo.MaterialInstances[x]->SetTextureParameterValue	("End",				EndTarget	);

			if (IsValid(TypesTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", TypesTarget);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", BlankMask);
			}
		}
		auto meshPtr = FoliageInstancers.Find(meshInfo.Mesh);

		if (meshPtr && IsValid(*meshPtr)) 
		{
			InstancedMesh = *meshPtr;
			InstancedMesh->SetWorldLocation(componentOffset);
			InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else 
		{
			auto instancerName	= FName(*("InstanceTrees" + FString::FromInt(FoliageInstancers.Num())));
			InstancedMesh		= NewObject<UHierarchicalInstancedStaticMeshComponent>(this, instancerName);

			InstancedMesh->RegisterComponent();
			InstancedMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			InstancedMesh->SetStaticMesh(meshInfo.Mesh);
			InstancedMesh->SetWorldLocation(componentOffset);
			InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FoliageInstancers	.Add(meshInfo.Mesh, InstancedMesh);
			ArrayOfInstancers	.Add(InstancedMesh);
			ArrayOfMeshInfos	.Add(meshInfo);
		}
		InstancedMesh->SetCastShadow(true);
		InstancedMesh->SetCullDistances(StartCullDistance, EndCullDistance);

		for (int x = 0; x < meshInfo.MaterialInstances.Num(); ++x)
		{
			UMaterialInterface* material = meshInfo.Mesh->GetMaterial(x);
			InstancedMesh->SetMaterial(x, meshInfo.MaterialInstances[x]);
		}

		for (int x = 0; x < meshInfo.MaterialInstances.Num(); ++x)
		{
			meshInfo.MaterialInstances[x]->SetVectorParameterValue("MeshComponentPosition", InstancedMesh->GetComponentLocation());
		}
	}

	switch (MeshLayersOption) 
	{
		case ELayersOption::PolyLayered: 
		{
			for (FFoliageMeshInfo meshInfo : FoliageMeshes)
			{
				TArray<FFoliageMeshInfo> infos;
				TArray<UHierarchicalInstancedStaticMeshComponent*> instancers;

				infos		.Add(meshInfo); 
				instancers	.Add(*FoliageInstancers.Find(meshInfo.Mesh));
				this->FillFoliage(infos, instancers, DebugDraw, updateMaskBuffer);
			}
			break;
		}
		case ELayersOption::MonoLayered: 
		{
			this->FillFoliage(ArrayOfMeshInfos, ArrayOfInstancers, DebugDraw, updateMaskBuffer);
			break;
		}
	}
}



void ACustomFoliageInstancer::FillFoliage(TArray<FFoliageMeshInfo> Infos, TArray<UHierarchicalInstancedStaticMeshComponent*> Instancers, bool DebugDraw, bool updateMaskBuffer)
{
	float cellX				= 0;
	float cellY				= 0;
	float density			= 0;
	float X					= 0;
	float Y					= 0;
	int totalAdded			= 0;
	float instancerAngle	= (SpawnAngle) * (PI / 180);

	FVector2D originPoint = FVector2D(Width / 2, Height / 2);
	FFoliageMeshInfo meshInfo;
	UHierarchicalInstancedStaticMeshComponent* instancedMesh;

	FRandomStream rs(InstancerSeed);
	while (cellX < Width)
	{
		while (cellY < Height)
		{
			int index		= FMath::RandRange(0, Infos.Num() - 1);
			meshInfo		= Infos[index];
			instancedMesh	= Instancers[index];
			density			= 0;
			while (density <= meshInfo.Density)
			{
				float helperX = 0.0f;
				float helperY = 0.0f;

				helperX = 1.0f - FMath::Clamp((cellX + CellSize.X - Width) / CellSize.X, 0.0f, 1.0f);
				helperY = 1.0f - FMath::Clamp((cellY + CellSize.Y - Height) / CellSize.Y, 0.0f, 1.0f);
				density += FMath::Clamp(rs.FRandRange(0.5f, 1.5f) / (helperX*helperY), 0.0f, 1.0f);

				auto scatterX = (1 - meshInfo.Scatter)*CellSize.X / 2;
				auto scatterY = (1 - meshInfo.Scatter)*CellSize.Y / 2;

				helperX = rs.FRandRange(FMath::Clamp(scatterX, 0.0f, (Width - cellX)),	FMath::Clamp(CellSize.X - scatterX, 0.0f, (Width - cellX))	);
				helperY = rs.FRandRange(FMath::Clamp(scatterY, 0.0f, (Height - cellY)), FMath::Clamp(CellSize.Y - scatterY, 0.0f, (Height - cellY))	);

				X = FMath::Clamp(cellX + helperX + SpawnTranslate.X, 0.0f, Width);
				Y = FMath::Clamp(cellY + helperY + SpawnTranslate.Y, 0.0f, Height);
				if (instancerAngle !=0)
				{
					auto newX = FMath::Cos(instancerAngle) * (X - originPoint.X) -
								FMath::Sin(instancerAngle) * (Y - originPoint.Y) + originPoint.X;
					auto newY = FMath::Sin(instancerAngle) * (X - originPoint.X) +
								FMath::Cos(instancerAngle) * (Y - originPoint.Y) + originPoint.Y;

					X = FMath::Clamp(newX, 0.0f, Width	);
					Y = FMath::Clamp(newY, 0.0f, Height	);
				}

				int px = X / Width * (InitialMask->GetSizeX()-1);
				int py = Y / Height * (InitialMask->GetSizeY()-1);

				if (ColorBuffer[(int)(py * InitialMask->GetSizeX() + px)].R>0) 
				{
					totalAdded++;

					if (!DebugDraw)
					{
						auto meshScale	= rs.FRandRange(meshInfo.MinScale, meshInfo.MaxScale);
						float meshAngle = SpawnAngle + meshInfo.RotationOrigin;
						FQuat rotation;
						if (meshInfo.UseRotationPresets && meshInfo.RotationPresets.Num() > 0)
						{
							int presetIndex = rs.RandRange(0, meshInfo.RotationPresets.Num() - 1);

							meshAngle	+= meshInfo.RotationPresets[presetIndex];
							rotation	= FQuat::MakeFromEuler(FVector(0.0f, 0.0f, meshAngle));
						}
						else
						{
							meshAngle	+= rs.FRandRange(-meshInfo.MaxRotation, meshInfo.MaxRotation);
							rotation	= FQuat::MakeFromEuler(FVector(0.0f, 0.0f, meshAngle));
						}
						instancedMesh->AddInstance(
							FTransform(
								rotation,
								FVector(X, Y, meshInfo.Z_AxisCorrection),
								FVector(meshScale)
							)
						);
						density++;
					}
				}
			}
			cellY += CellSize.Y;
		}
		cellX += CellSize.X; cellY = 0;
	}
}


void ACustomFoliageInstancer::UpdateBuffer()
{
	ColorBuffer.Reset();

	if (InitialTarget != NULL)
	{
		FTextureRenderTarget2DResource* textureResource = (FTextureRenderTarget2DResource*)InitialTarget->Resource;
		if (textureResource->ReadPixels(ColorBuffer))
		{
		}
	}
}

FColor ACustomFoliageInstancer::GetRenderTargetValue(float x, float y)
{
	float size = 10000;

	if (InitialTarget == NULL || ColorBuffer.Num() == 0)
	{
		return FColor(0);
	}

	float width		= InitialTarget->GetSurfaceWidth();
	float height	= InitialTarget->GetSurfaceHeight();

	//Conver coordinates to texture space
	float normalizedX = (x / size) + 0.5f;
	float normalizedY = (y / size) + 0.5f;

	int i = (int)(normalizedX * width);
	int j = (int)(normalizedY * height);

	if (i < 0) i = 0;
	if (i >= width) i = width - 1;
	if (j < 0) j = 0;
	if (j >= height) j = height - 1;

	int index = i + j * width;
	if (index < 0) index = 0;
	if (index >= ColorBuffer.Num()) index = ColorBuffer.Num();

	return ColorBuffer[index];
}