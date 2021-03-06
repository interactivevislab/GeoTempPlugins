#include "BuildingSpawner.h"
#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "BuildingActor.h"
#include "BuildingsData.h"

void UBuildingSpawner::SpawnBuildingActors(const TArray<FBuilding>& inBuildingData, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial)
{
	TSet<long> usedParts;

	for (auto building : inBuildingData)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = GetOwner();
		SpawnInfo.Name = FName(*("Building_" + FString::FromInt(building.Id)));

		auto buildingActor = GetWorld()->SpawnActor<ABuildingActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
		buildingActor->SetActorLabel(SpawnInfo.Name.ToString());

		buildingActor->WallMaterial = UMaterialInstanceDynamic::Create(inWallMaterial, buildingActor);
		buildingActor->RoofMaterial = UMaterialInstanceDynamic::Create(inRoofMaterial, buildingActor);
		buildingActor->Initialize(building);

		buildingActor->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);

		Buildings.Add(buildingActor);
	}
}

void UBuildingSpawner::CleanBuildings() {

	for (auto building : this->Buildings) 
	{
		if (building) 
		{
			building->Destroy();
		}
	}
	
	AActor* owner = GetOwner();
	TArray<ABuildingActor*> toDestroy;
	
	for (auto child : owner->Children)
	{
		auto castChild = Cast<ABuildingActor>(child);
		if (castChild)
		{
			toDestroy.Add(castChild);
		}
	}
	for (auto child : toDestroy)
	{
		child->Destroy();
	}
	
	Buildings.Empty();
}
