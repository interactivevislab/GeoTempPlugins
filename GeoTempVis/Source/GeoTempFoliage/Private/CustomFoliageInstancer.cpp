// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomFoliageInstancer.h"
#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"


bool IsPointInPolygon(TArray<FVector> simPoly, FVector point)
{
	int j = simPoly.Num() - 1;
	bool oddNodes = false;
	for (int i = 0; i < simPoly.Num(); i++)
	{
		if ((simPoly[i].Y < point.Y && simPoly[j].Y >= point.Y || simPoly[j].Y < point.Y && simPoly[i].Y >= point.Y) && (simPoly[i].X <= point.X || simPoly[j].X <= point.X))
		{
			oddNodes ^= (simPoly[i].X + (point.Y - simPoly[i].Y) / (simPoly[j].Y - simPoly[i].Y) * (simPoly[j].X - simPoly[i].X) < point.X);
		}
		j = i;
	}

	return oddNodes;
}


UCustomFoliageInstancer::UCustomFoliageInstancer()
{
	Width				= 300;
	Height				= 300;
	CellSize.X			= 20;
	CellSize.Y			= 20;
	SpawnTranslate.X	= 0;
	SpawnTranslate.Y	= 0;
	SpawnAngle			= 0;
	MeshLayersOption	= ELayersOption::PolyLayered;

	ObjectQuery.Add(EObjectTypeQuery::ObjectTypeQuery1);

	updateSecondRenderTarget	= false;
	currentMaskIndex			= 0;
	polygonDates				= TArray<int>();

	DataBaseTags = 
	{
		{"AppearStart","AppearStart" },
		{"AppearEnd","AppearEnd" },
		{"DemolishStart","DemolStart" },
		{"DemolishEnd","DemolEnd" },
		{"Category","typeRole" },
		{"CategoryForest","forest" },
		{"CategoryPark","park" },
		{"CategoryYard","yard" },
		{"Alt","Aftergrow" },
		{"AltValue","True" },
		{"Exclude","Type" },
		{"ExcludeValue","Exclude" }
	};
}


void UCustomFoliageInstancer::BeginPlay()
{
	Super::BeginPlay();
}


void UCustomFoliageInstancer::InterpolateFoliageWithMaterial()
{
	for (FFoliageMeshInfo& Info : FoliageMeshes)
	{
		auto meshPtr = FoliageInstancers.FindRef(Info.Mesh);

		if (!meshPtr || !IsValid(meshPtr))
		{
			return;
		}
		meshPtr->SetScalarParameterValueOnMaterials("Interpolation", CurrentInterpolation);
	}
}


void UCustomFoliageInstancer::FillFoliageWithMask_BP(FVector4 inComponentRect)
{
	Width = Height = inComponentRect.Y - inComponentRect.X;


	FVector componentOffset		= FVector(inComponentRect.X, inComponentRect.Z, 0);

	TArray<FFoliageMeshInfo> arrayOfMeshInfos = {};
	TArray<UHierarchicalInstancedStaticMeshComponent*> arrayOfInstancers = {};

	UpdateBuffer();
	PrepareInstancers(componentOffset, arrayOfMeshInfos, arrayOfInstancers);
	/*for (FFoliageMeshInfo& meshInfo : FoliageMeshes)
	{
		UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

		for (int x = 0; x < meshInfo.Mesh->StaticMaterials.Num(); ++x)
		{
			UMaterialInterface* material = meshInfo.Mesh->GetMaterial(x);

			if (material->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				meshInfo.MaterialInstances.Add(x, Cast<UMaterialInstanceDynamic>(material));
			}
			else
			{
				auto dynamicMaterial = UMaterialInstanceDynamic::Create(material, this);
				meshInfo.MaterialInstances.Add(x, dynamicMaterial);
			}
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerWidth",	Width		);
			meshInfo.MaterialInstances[x]->SetScalarParameterValue	("InstancerHeight", Height		);

			if (IsValid(StartTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Start", StartTarget	);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Start", InitialTarget	);
			}

			if (IsValid(EndTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("End", EndTarget		);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("End", InitialTarget	);
			}

			if (IsValid(TypesTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", TypesTarget		);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", InitialTarget	);
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
			if (!foliageActor)
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.Owner = GetOwner();
				SpawnInfo.Name = "FoliageActor";
				foliageActor = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
			}


			auto instancerName	= FName(*("InstanceTrees" + FString::FromInt(FoliageInstancers.Num())));
			AActor* owner = this->GetOwner();

			InstancedMesh	= NewObject<UHierarchicalInstancedStaticMeshComponent>(foliageActor, instancerName);
			foliageActor	->AddInstanceComponent(InstancedMesh);
			InstancedMesh	->RegisterComponent();

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
			meshInfo.MaterialInstances[x]->SetScalarParameterValue("Interpolation", 0.0f);
		}
	}*/

	switch (MeshLayersOption) 
	{
		case ELayersOption::PolyLayered: 
		{
			for (FFoliageMeshInfo& meshInfo : FoliageMeshes)
			{
				TArray<FFoliageMeshInfo> infos;
				TArray<UHierarchicalInstancedStaticMeshComponent*> instancers;

				infos		.Add(meshInfo); 
				instancers	.Add(*FoliageInstancers.Find(meshInfo.Mesh));

				this->FillFoliageWithMeshes(infos, instancers);
			}
			break;
		}
		case ELayersOption::MonoLayered: 
		{
			this->FillFoliageWithMeshes(arrayOfMeshInfos, arrayOfInstancers);
			break;
		}
	}
}


void UCustomFoliageInstancer::FillFoliageWithPolygons_BP(TArray<FContourData> inPolygons)
{

	FVector componentOffset = FVector::ZeroVector;

	TArray<FFoliageMeshInfo> arrayOfMeshInfos = {};
	TArray<UHierarchicalInstancedStaticMeshComponent*> arrayOfInstancers = {};

	PrepareInstancers(componentOffset, arrayOfMeshInfos, arrayOfInstancers);

	//{
		TArray<FContourData> includePolys = {};
		TArray<FContour> excludePolys = {};
		for (auto poly: inPolygons)
		{
			if (poly.Tags.Find("typeRole"))
			{
				//for (auto outer : poly.Outer)
				//{
					includePolys.Add(poly);
				//}
				for (auto inner : poly.Holes)
				{
					excludePolys.Add(inner);
				}
			}
			else
			{
				for (auto outer : poly.Outer)
				{
					excludePolys.Add(outer);
				}
			}
		}
		//for (auto include : includePolys)
		//{

		//}

	for (auto include : includePolys)
	{
		for (auto outer : include.Outer)
		{
			TArray<FContour> exclude = {};

			for (auto excludePoly : excludePolys)
			{
				for (auto point : excludePoly.Points)
				{
					if (IsPointInPolygon(outer.Points, point))
					{
						exclude.Add(excludePoly);
						break;
					}
				}
			}


			TArray<FFoliageMeshInfo> infos;
			TArray<UHierarchicalInstancedStaticMeshComponent*> instancers;

			auto leafTag = include.Tags.Find("leaf_type");

			if (!leafTag || leafTag->Equals("mixed"))
			{
				infos = arrayOfMeshInfos;
				instancers = arrayOfInstancers;
			}
			else
			{
				auto leafType = leafTag->Equals("broadleaved") ? ELeafType::Broadleaved : ELeafType::Needleleaved;
				for (FFoliageMeshInfo& meshInfo : FoliageMeshes)
				{
					if (meshInfo.LeafType == leafType)
					{
						infos.Add(meshInfo);
						instancers.Add(*FoliageInstancers.Find(meshInfo.Mesh));
					}
				}
			}

			this->FillFoliageByPolygon(outer.Points, exclude, infos, instancers);
		}
	}
}


void UCustomFoliageInstancer::PrepareInstancers(
	FVector inComponentOffset, 
	TArray<FFoliageMeshInfo>& outArrayOfMeshInfos, 
	TArray<UHierarchicalInstancedStaticMeshComponent*>& outArrayOfInstancers
)
{
	const int StartCullDistance	= 2000000000;
	const int EndCullDistance	= 2100000000;

	for (FFoliageMeshInfo& meshInfo : FoliageMeshes)
	{
		UHierarchicalInstancedStaticMeshComponent* InstancedMesh;

		for (int x = 0; x < meshInfo.Mesh->StaticMaterials.Num(); ++x)
		{
			UMaterialInterface* material = meshInfo.Mesh->GetMaterial(x);

			if (material->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				meshInfo.MaterialInstances.Add(x, Cast<UMaterialInstanceDynamic>(material));
			}
			else
			{
				auto dynamicMaterial = UMaterialInstanceDynamic::Create(material, this);
				meshInfo.MaterialInstances.Add(x, dynamicMaterial);
			}
			meshInfo.MaterialInstances[x]->SetScalarParameterValue("InstancerWidth", Width);
			meshInfo.MaterialInstances[x]->SetScalarParameterValue("InstancerHeight", Height);

			if (IsValid(StartTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Start", StartTarget);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Start", InitialTarget);
			}

			if (IsValid(EndTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("End", EndTarget);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("End", InitialTarget);
			}

			if (IsValid(TypesTarget))
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", TypesTarget);
			}
			else
			{
				meshInfo.MaterialInstances[x]->SetTextureParameterValue("Type", InitialTarget);
			}
		}
		auto meshPtr = FoliageInstancers.Find(meshInfo.Mesh);

		if (meshPtr && IsValid(*meshPtr))
		{
			InstancedMesh = *meshPtr;
			InstancedMesh->SetWorldLocation(inComponentOffset);
			InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			if (!foliageActor)
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.Owner = GetOwner();
				SpawnInfo.Name = "FoliageActor";
				foliageActor = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
			}


			auto instancerName = FName(*("InstanceTrees" + FString::FromInt(FoliageInstancers.Num())));
			AActor* owner = this->GetOwner();

			InstancedMesh = NewObject<UHierarchicalInstancedStaticMeshComponent>(foliageActor, instancerName);
			foliageActor->AddInstanceComponent(InstancedMesh);
			InstancedMesh->RegisterComponent();

			InstancedMesh->SetStaticMesh(meshInfo.Mesh);
			InstancedMesh->SetWorldLocation(inComponentOffset);
			InstancedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FoliageInstancers.Add(meshInfo.Mesh, InstancedMesh);
			outArrayOfInstancers.Add(InstancedMesh);
			outArrayOfMeshInfos.Add(meshInfo);
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
			meshInfo.MaterialInstances[x]->SetScalarParameterValue("Interpolation", 0.0f);
		}
	}
}

void UCustomFoliageInstancer::FillFoliageWithMeshes(
	TArray<FFoliageMeshInfo>& inInfos, 
	TArray<UHierarchicalInstancedStaticMeshComponent*>& inInstancers)
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
			int index		= FMath::RandRange(0, inInfos.Num() - 1);
			meshInfo		= inInfos[index];
			instancedMesh	= inInstancers[index];
			density			= 0;
			while (density <= meshInfo.Density)
			{
				float helperX = 0.0f;
				float helperY = 0.0f;

				helperX = 1.0f - FMath::Clamp((cellX + CellSize.X - Width)	/ CellSize.X, 0.0f, 1.0f);
				helperY = 1.0f - FMath::Clamp((cellY + CellSize.Y - Height)	/ CellSize.Y, 0.0f, 1.0f);

				density +=FMath::Clamp(rs.FRandRange(0.5f, 1.5f) / (helperX*helperY), 0.0f, 1.0f);

				auto scatterX = (1 - meshInfo.Scatter)*CellSize.X / 2;
				auto scatterY = (1 - meshInfo.Scatter)*CellSize.Y / 2;

				helperX = rs.FRandRange(FMath::Clamp(scatterX, 0.0f, (Width - cellX)),	FMath::Clamp(CellSize.X - scatterX, 0.0f, (Width - cellX))	);
				helperY = rs.FRandRange(FMath::Clamp(scatterY, 0.0f, (Height - cellY)),	FMath::Clamp(CellSize.Y - scatterY, 0.0f, (Height - cellY))	);

				X = FMath::Clamp(cellX + helperX + SpawnTranslate.X, 0.0f, Width);
				Y = FMath::Clamp(cellY + helperY + SpawnTranslate.Y, 0.0f, Height);
				if (instancerAngle !=0)
				{
					auto newX	=	FMath::Cos(instancerAngle) * (X - originPoint.X) -
									FMath::Sin(instancerAngle) * (Y - originPoint.Y) + originPoint.X;
					auto newY	=	FMath::Sin(instancerAngle) * (X - originPoint.X) +
									FMath::Cos(instancerAngle) * (Y - originPoint.Y) + originPoint.Y;

					X = FMath::Clamp(newX, 0.0f, Width	);
					Y = FMath::Clamp(newY, 0.0f, Height	);
				}

				int px = X / Width	* (InitialTarget->GetSurfaceWidth()	-1);
				int py = Y / Height	* (InitialTarget->GetSurfaceHeight()-1);

				float spawnRandomValue	= rs.FRandRange(0.0f, 1.0f);
				float presenceChance	= colorBuffer[(int)(py * InitialTarget->GetSurfaceWidth() + px)].R/255.0f;
				bool presenceSuppressed	= colorBuffer[(int)(py * InitialTarget->GetSurfaceWidth() + px)].G > 0;

				if	(!presenceSuppressed && presenceChance > 0 && spawnRandomValue <= presenceChance)
				{
					totalAdded++;

					auto meshScale	= rs.FRandRange(meshInfo.MinScale, meshInfo.MaxScale);
					float meshAngle	= SpawnAngle + meshInfo.RotationOrigin;
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
			cellY += CellSize.Y;
		}
		cellX += CellSize.X; 
		cellY = 0;
	}
}


void UCustomFoliageInstancer::FillFoliageByPolygon(
	TArray<FVector> inPolygon,
	TArray<FContour> inExcludePolygons,
	TArray<FFoliageMeshInfo>& inInfos,
	TArray<UHierarchicalInstancedStaticMeshComponent*>& inInstancers
)
{
	float minX, maxX, minY, maxY;

	int AttemptTries = 30;

	float cellX = 0;
	float cellY = 0;
	float density = 0;
	float X = 0;
	float Y = 0;
	int totalAdded = 0;
	float instancerAngle = (SpawnAngle) * (PI / 180);

	FVector2D originPoint = FVector2D(Width / 2, Height / 2);
	FFoliageMeshInfo meshInfo;
	UHierarchicalInstancedStaticMeshComponent* instancedMesh = nullptr;

	FRandomStream rs(InstancerSeed);

	float helper1 = 0.0f, helper2 = 0.0f;

		cellX = 0;
		cellY = 0;

		minX = minY = MAX_FLT;
		maxX = maxY = MAX_FLT * -1.0f;

		TArray<FVector2D> correctedPolygon;
		X =0;
		Y =0;
		for (int i = 0; i < inPolygon.Num(); i++)
		{

			X = inPolygon[i].X;
			Y = inPolygon[i].Y;
			correctedPolygon.Add(FVector2D(X, Y));
			if (X < minX)
				minX = X;
			if (Y < minY)
				minY = Y;
			if (X > maxX)
				maxX = X;
			if (Y > maxY)
				maxY = Y;
		}
		cellX = minX;
		cellY = minY;
		while (cellX < maxX)
		{
			while (cellY < maxY)
			{
				int index = FMath::RandRange(0, inInfos.Num() - 1);
				meshInfo = inInfos[index];
				instancedMesh = inInstancers[index];
				density = 0;


				helper1 = 0.0f, helper2 = 0.0f;

				helper1 = FMath::FRandRange(0.0f, FMath::Clamp(CellSize.X, 0.0f, (maxX - cellX)));
				helper2 = FMath::FRandRange(0.0f, FMath::Clamp(CellSize.Y, 0.0f, (maxY - cellY)));

				X = cellX + helper1;
				Y = cellY + helper2;

				bool treeSupressed = false;
				for (auto exclude : inExcludePolygons)
				{
					if (IsPointInPolygon(exclude.Points, FVector(X, Y, 0)))
					{
						treeSupressed = true;
						break;
					}
				}

				if (!treeSupressed && IsPointInPolygon(inPolygon, FVector(X, Y, 0)))
				{

					FVector vHelper1, vHelper2;

					vHelper1 = FVector(X, Y,  10000) + instancedMesh->GetComponentLocation();
					vHelper2 = FVector(X, Y, -10000) + instancedMesh->GetComponentLocation();

					FHitResult hitResult;

					//UKismetSystemLibrary::LineTraceSingleForObjects(
					//	this->GetWorld(),
					//	vHelper1,
					//	vHelper2,
					//	ObjectQuery,
					//	true,
					//	TArray<AActor*>(),
					//	Test ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
					//	hitResult,
					//	true);
					if (true/*hitResult.Location.Z <= -4 || !checkForCollisions*/)
					{
						auto meshScale = rs.FRandRange(meshInfo.MinScale, meshInfo.MaxScale);

						auto rotation = FQuat::MakeFromEuler(FVector(0.0f, 0.0f, FMath::FRandRange(0.0f, 360.0f)));

						instancedMesh->AddInstance(
							FTransform(
								rotation,
								FVector(X, Y, -2),
								FVector(meshScale)
							)
						);
					}
				}
				cellY += CellSize.Y;
			}
			cellX += CellSize.X;
			cellY = minY;
		}
	
}


void UCustomFoliageInstancer::ClearFoliage_BP()
{
	AActor* owner = this->GetOwner();
	if (!foliageActor)
	{
		return;
	}
	for (auto& FoliageInstance : FoliageInstancers)
	{
		FoliageInstance.Value->ClearInstances();
		FoliageInstance.Value->UnregisterComponent();
		foliageActor->RemoveInstanceComponent(FoliageInstance.Value);
		FoliageInstance.Value->DestroyComponent();
	}
	FoliageInstancers.Empty();
}


void UCustomFoliageInstancer::UpdateFoliageMasksDates(FDateTime inCurrentTime, int& outRenderYearFirst, int& outRenderYearSecond, bool& outUpdateFirstTarget)
{
	if (polygonDates.Num()==0)
	{
		return;
	}
	int firstMaskYear	= FMath::Max(polygonDates[currentMaskIndex],	1);
	int secondMaskYear	= FMath::Max(polygonDates[currentMaskIndex+1],	1);
	FDateTime firstMaskDate		= FDateTime(firstMaskYear,	1, 1);
	FDateTime secondMaskDate	= FDateTime(secondMaskYear,	1, 1);

	if (!updateSecondRenderTarget)
	{
		float secondsAfterFirstMask		= (inCurrentTime	- firstMaskDate	).GetTotalSeconds();
		float secondsBetweenMasks		= (secondMaskDate	- firstMaskDate	).GetTotalSeconds();
		float secondsBeforeSecondMask	= (secondMaskDate	- inCurrentTime	).GetTotalSeconds();

		CurrentInterpolation = FMath::Clamp(secondsAfterFirstMask / secondsBetweenMasks, 0.0f, 1.0f);

		if (secondsAfterFirstMask < 0 || secondsBeforeSecondMask < 0)
		{
			GetDatesNearCurrent(inCurrentTime);
		}
	}

	outRenderYearFirst		= polygonDates[currentMaskIndex		];
	outRenderYearSecond		= polygonDates[currentMaskIndex + 1	];
	outUpdateFirstTarget	= CurrentInterpolation >= 1.0f;

	if (!updateSecondRenderTarget)
	{
		CurrentInterpolation = CurrentInterpolation >= 1.0f ? 0.0f : 1.0f;
	}
	updateSecondRenderTarget = !updateSecondRenderTarget;
}


void UCustomFoliageInstancer::GetDatesNearCurrent(FDateTime inCurrentTime)
{
	int currentMaskIndexLocal = -1;

	for (int i = 0; i < polygonDates.Num()-1; i++)
	{
		auto indexedMaskDate			= FDateTime(FMath::Max(polygonDates[i], 1),1,1);
		auto secondsFromCurrentMaskDate	= (indexedMaskDate - inCurrentTime).GetTotalSeconds();
		if (secondsFromCurrentMaskDate <= 0.0f)
		{
			currentMaskIndexLocal++;
		}
		else
		{
			currentMaskIndexLocal = FMath::Max(currentMaskIndexLocal,0);
			break;
		}
	}
	currentMaskIndex = FMath::Min(currentMaskIndexLocal, polygonDates[polygonDates.Num() - 2]);
}


void UCustomFoliageInstancer::ParseDates(TArray<FContourData>& inContours)
{
	TSet<int> maskDates = TSet<int>();

	for (int i = 0; i < inContours.Num()-1; i++)
	{
		ParseTimeTags(inContours[i], maskDates);
	}
	SortDatesByAscend(maskDates);
}


void UCustomFoliageInstancer::ParseTimeTags(const FContourData& inContour, TSet<int>& outDates)
{
	int date	= inContour.Tags.Find("AppearStart")	? FCString::Atoi(**inContour.Tags.Find("AppearStart")		):0;
	outDates.Add(date);
	date		= inContour.Tags.Find("DemolishStart")	? FCString::Atoi(**inContour.Tags.Find("DemolishStart")		):1;
	outDates.Add(date);
}


void UCustomFoliageInstancer::SortDatesByAscend(TSet<int>& inDates)
{
	int minDate = 0;
	auto datesArray = inDates.Array();
	while (datesArray.Num()>0)
	{
		minDate = datesArray[0];
		for (int i = 0; i < datesArray.Num(); i++)
		{
			if (datesArray[i] < minDate)
			{
				minDate = datesArray[i];
			}
		}
		polygonDates.Add(minDate);
		datesArray.Remove(minDate);
	}
}


void UCustomFoliageInstancer::UpdateBuffer()
{
	colorBuffer.Reset();

	if (InitialTarget != NULL)
	{
		auto textureResource = InitialTarget->GameThread_GetRenderTargetResource();
		FReadSurfaceDataFlags flags;
		flags.SetLinearToGamma(false);
		textureResource->ReadPixels(colorBuffer,flags);
	}
}
