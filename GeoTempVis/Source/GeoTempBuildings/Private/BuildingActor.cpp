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


void ABuildingActor::Initialize(FBuilding* building, bool initPartsImmideately)
{
	if (RootComponent == nullptr) {
		RootComponent = NewObject<USceneComponent>(this, TEXT("RootComponent"));
		FinishAndRegisterComponent(RootComponent);
	}
	Building = building;

	for (auto& cont : Building->MainPart->OuterConts) {
		Outer.Add(cont);
	}
	for (auto& cont : Building->MainPart->InnerConts) {
		Inner.Add(cont);
	}
	for (auto& part : Parts) {
		part->DestroyComponent();
	}
	Id = Building->Id;
	Parts.Empty();
	BuildingTags = Building->Tags;
	if (Building->Parts.size() == 0) {
		auto name = FName(*(FString::FromInt(Building->Id) + "_" + FString::FromInt(Building->MainPart->Id)));
		UBuildingPartComponent* part = NewObject<UBuildingPartComponent>(this, name);
		
		part->OnComponentCreated();
		
		part->SetupAttachment(RootComponent);
		part->RegisterComponent();
		this->AddInstanceComponent(part);

		if (part)
		{
			FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
			part->WallMaterial = WallMaterial;
			part->RoofMaterial = RoofMaterial;			
			part->Init(Building->MainPart, Building->Tags);
			if (initPartsImmideately) part->ReInit();
			part->StylePalette = Building->MainPart->StylePalette;
			Parts.Add(part);
		}
	}
	else {
		for (auto& partData : Building->Parts) {
			auto name = FName(*(FString::FromInt(Building->Id) + "_" + FString::FromInt(partData->Id)));
			UBuildingPartComponent* part = NewObject<UBuildingPartComponent>(this, name);
			part->RegisterComponent();
			part->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			if (part)
			{
				FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, false);
				part->WallMaterial = WallMaterial;
				part->RoofMaterial = RoofMaterial;				
				part->Init(partData, Building->Tags);
				if (initPartsImmideately) part->ReInit();
				part->StylePalette = partData->StylePalette;
				Parts.Add(part);
			}
		}
	}
}

void ABuildingActor::ReInitialize() { Initialize(this->Building); }