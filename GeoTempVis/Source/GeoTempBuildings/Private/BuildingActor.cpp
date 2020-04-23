#include "BuildingActor.h"
#include "BuildingPart.h"
#include "Contour.h"

ABuildingActor::ABuildingActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABuildingActor::OnConstruction(const FTransform& Transform)
{
	for (auto& part : Parts) {
		part->Recalc();
	}
} 


void ABuildingActor::Initialize(const FBuilding& inBuilding, bool inInitPartsImmideately)
{
	if (RootComponent == nullptr)
	{
		RootComponent = NewObject<USceneComponent>(this, TEXT("RootComponent"));
		FinishAndRegisterComponent(RootComponent);
	}
	Building = inBuilding;

	for (auto& cont : Building.MainPart.OuterConts)
	{
		Outer.Add(cont);
	}
	for (auto& cont : Building.MainPart.InnerConts)
	{
		Inner.Add(cont);
	}
	
	for (auto& part : Parts)
	{
		part->DestroyComponent();
	}
	
	Id = Building.Id;
	Parts.Empty();
	BuildingTags = Building.Tags;
	
	if (Building.Parts.Num() == 0 || Building.MainPart.Height != 0) 
	{
		auto name = FName(*(FString::FromInt(Building.Id) + "_" + FString::FromInt(Building.MainPart.Id)));
		UBuildingPartComponent* part = NewObject<UBuildingPartComponent>(this, name);
		if (part) {
			part->OnComponentCreated();
			part->SetupAttachment(RootComponent);
			part->RegisterComponent();
			this->AddInstanceComponent(part);
		
			FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
			part->WallMaterial = WallMaterial;
			part->RoofMaterial = RoofMaterial;
			part->Init(Building.MainPart);
			if (inInitPartsImmideately) part->ForceRecalc();
			part->StylePalette = Building.MainPart.StylePalette;
			Parts.Add(part);
		}
	}

	for (auto& partData : Building.Parts) 
	{
		auto name = FName(*(FString::FromInt(Building.Id) + "_" + FString::FromInt(partData.Id)));
		UBuildingPartComponent* part = NewObject<UBuildingPartComponent>(this, name);

		if (part)
		{
			part->RegisterComponent();
			part->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			
			FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
			part->WallMaterial = WallMaterial;
			part->RoofMaterial = RoofMaterial;
			part->Init(partData);
			
			if (inInitPartsImmideately) part->ForceRecalc();
			part->StylePalette = partData.StylePalette;
			Parts.Add(part);
		}
	}
}

void ABuildingActor::ReInitialize()
{
	Initialize(this->Building);
}
