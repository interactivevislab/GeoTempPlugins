#include "BuildingPart.h"
#include "BuildingUtils.h"

UBuildingPartComponent::UBuildingPartComponent(const FObjectInitializer& ObjectInitializer) : URuntimeMeshComponent(ObjectInitializer)
{
}

void UBuildingPartComponent::Actualize()
{
}

float UBuildingPartComponent::SimplifyDistance = 250000;

void UBuildingPartComponent::Init(FBuildingPart* inPart, TMap<FString, FString> inTags/*, UBuildingsLoaderBase2* owner*/) {
	for (auto& cont : inPart->OuterConts) 
	{
		cont.FixClockwise();
		Outer.Add(cont);
	}
	for (auto& cont : inPart->InnerConts) 
	{
		cont.FixClockwise(false);
		Inner.Add(cont);
	}
	Floors			= inPart->Floors;
	Height			= inPart->Height;
	MinFloors		= inPart->MinFloors;
	MinHeight		= inPart->MinHeight;
	BuildingDates	= inPart->BuildingDates;
	
	if (Floors == 0)
	{
		MinFloors = -1;
		inPart->MinFloors = -1;
	}

	Id = inPart->Id;
	PartData = inPart;
	Tags = inTags;
}

void UBuildingPartComponent::ReInit()
{
	_isInit = false;
	Recalc();
	_isInit = true;
}


void UBuildingPartComponent::Recalc() {	
	if (!OverrideHeight)
	{
		Height = 0;
		MinHeight = 0;
	}
	
	if (AutoUpdateMesh || !_isInit)
	{
		CreateSimpleStructure();
	}
	
	SetRelativeLocation(Center);
}


void UBuildingPartComponent::CreateSimpleStructure(float inZeroHeight)
{	
	if (PartData)
	{	
		auto meshData = MeshHelpers::CalculateMeshData(PartData, 0, WallMaterial, RoofMaterial);
		MeshHelpers::ConstructRuntimeMesh(this, meshData);
		for (int i = 0; i < GetNumSections(); i++)
		{
			SetMeshSectionCollisionEnabled(i, true);
		}
	}
}

void UBuildingPartComponent::SetHeightAlpha(float inHeightAlpha)
{
	SetRelativeLocation(FVector(0, 0, (350 * Floors + 150) * (inHeightAlpha - 1)));
}


void UBuildingPartComponent::ApplyCurrentTime(FDateTime inCurrentTime, bool inMustHide)
{
	bool isDemolished = (inCurrentTime - BuildingDates.DemolishStart).GetTotalDays() > 0;
	float daysFromBuildStart	=				(inCurrentTime				- BuildingDates.BuildStart).GetTotalDays();
	float buildDuration			= FMath::Max(	(BuildingDates.BuildEnd	- BuildingDates.BuildStart).GetTotalDays(), 1.0);

	SetHeightAlpha(FMath::Clamp(daysFromBuildStart / buildDuration, -1.0f, 1.0f));

	bool isVisible = !inMustHide && !isDemolished && (daysFromBuildStart >= -100);
	SetVisibility(isVisible, true);
	SetCollisionEnabled(isVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}


TArray<FLinearColor> UBuildingPartComponent::AllColors = TArray<FLinearColor>{};

void UBuildingPartComponent::SetAllColors(TArray<FLinearColor>& inColors)
{
	AllColors = inColors;
}
