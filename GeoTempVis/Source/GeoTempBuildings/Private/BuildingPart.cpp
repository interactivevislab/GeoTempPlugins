#include "BuildingPart.h"
#include "BuildingUtils.h"

UBuildingPartComponent::UBuildingPartComponent(const FObjectInitializer& ObjectInitializer) : URuntimeMeshComponent(ObjectInitializer)
{	
}

void UBuildingPartComponent::Actualize() {		
}

float UBuildingPartComponent::SimplifyDistance = 250000;

inline void UBuildingPartComponent::Init(FBuildingPart* part, TMap<FString, FString> tags/*, UBuildingsLoaderBase2* owner*/) {
	for (auto& cont : part->OuterConts) {
		cont.FixClockwise();
		Outer.Add(cont);
	}
	for (auto& cont : part->InnerConts) {
		cont.FixClockwise(false);
		Inner.Add(cont);
	}
	Floors = part->Floors;
	Height = part->Height;
	MinFloors = part->MinFloors;
	MinHeight = part->MinHeight;
	BuildingDates = part->BuildingDates;
	if (Floors == 0) {
		MinFloors = -1;
		part->MinFloors = -1;
	}

	Id = part->Id;
	partData = part;
	Tags = tags;

	//Owner = owner;
	//if (owner)	owner->BuildingsMap.Add(Id, this);
}

void UBuildingPartComponent::ReInit()
{
	_isInit = false;
	Recalc();
	_isInit = true;
}


void UBuildingPartComponent::Recalc() {	
	if (!OverrideHeight) {
		Height = 0;
		MinHeight = 0;
	}
	if (AutoUpdateMesh || !_isInit) {
		CreateSimpleStructure();				
	}
	
	SetRelativeLocation(Center);
}


void UBuildingPartComponent::CreateSimpleStructure(float zeroH)
{	
	if (partData) {	
		auto meshData = MeshHelpers::CalculateMeshData(partData, 0, WallMaterial, RoofMaterial);
		MeshHelpers::ConstructRuntimeMesh(this, meshData);
		for (int i = 0; i < GetNumSections(); i++) {
			SetMeshSectionCollisionEnabled(i, true);
		}
	}
}

void UBuildingPartComponent::SetHeightAlpha(float HeightAlpha) {
	SetRelativeLocation(FVector(0, 0, (350 * Floors + 150) * (HeightAlpha - 1)));
}


void UBuildingPartComponent::ApplyCurrentTime(FDateTime CurrentTime, bool MustHide)
{
	bool isDemolished = (CurrentTime - BuildingDates.DemolishStart).GetTotalDays() > 0;
	float daysFromBuildStart = (CurrentTime - BuildingDates.BuildStart).GetTotalDays();
	float buildDuration = FMath::Max((BuildingDates.BuildEnd - BuildingDates.BuildStart).GetTotalDays(), 1.0);

	SetHeightAlpha(FMath::Clamp(daysFromBuildStart / buildDuration, -1.0f, 1.0f));

	bool isVisible = !MustHide && !isDemolished && (daysFromBuildStart >= -100);
	SetVisibility(isVisible, true);
	SetCollisionEnabled(isVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}


TArray<FLinearColor> UBuildingPartComponent::AllColors = TArray<FLinearColor>{};

void UBuildingPartComponent::SetAllColors(TArray<FLinearColor>& colors) {
	AllColors = colors;
}
