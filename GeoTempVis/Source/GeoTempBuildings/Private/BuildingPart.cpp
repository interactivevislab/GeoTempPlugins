#include "BuildingPart.h"
#include "BuildingActor.h"
#include "BuildingUtils.h"

UBuildingPartComponent::UBuildingPartComponent(const FObjectInitializer& ObjectInitializer) : URuntimeMeshComponent(ObjectInitializer)
{
}

void UBuildingPartComponent::Init(const FBuildingPart& inPart) {


    Parent = Cast<ABuildingActor>(GetOwner());
    Outer.Empty();
    Inner.Empty();
    Outer.Append(inPart.OuterConts);
    Inner.Append(inPart.InnerConts);
    
    Floors            = inPart.Floors;
    Height            = inPart.Height;
    MinFloors        = inPart.MinFloors;
    MinHeight        = inPart.MinHeight;
    BuildingDates    = inPart.BuildingDates;
    
    Id = inPart.Id;
    PartData = inPart;
    Tags = inPart.Tags;
}

void UBuildingPartComponent::ForceRecalc()
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

    auto meshData = MeshHelpers::CalculateMeshData(Parent->Building, PartData, WallMaterial, RoofMaterial);
    MeshHelpers::ConstructRuntimeMesh(this, meshData);
    for (int i = 0; i < GetNumSections(); i++)
    {
        SetMeshSectionCollisionEnabled(i, true);
    }    
}

void UBuildingPartComponent::SetHeightAlpha(float inHeightAlpha)
{
    SetRelativeLocation(FVector(0, 0, (350 * Floors + 150) * (inHeightAlpha - 1)));
}


void UBuildingPartComponent::ApplyCurrentTime(FDateTime inCurrentTime, bool inMustHide)
{
    bool isDemolished = (inCurrentTime - BuildingDates.DemolishStart).GetTotalDays() > 0;
    float daysFromBuildStart    =                (inCurrentTime                - BuildingDates.BuildStart).GetTotalDays();
    float buildDuration            = FMath::Max(    (BuildingDates.BuildEnd    - BuildingDates.BuildStart).GetTotalDays(), 1.0);

    SetHeightAlpha(FMath::Clamp(daysFromBuildStart / buildDuration, -1.0f, 1.0f));

    bool isVisible = !inMustHide && !isDemolished && (daysFromBuildStart >= -100);
    SetVisibility(isVisible, true);
    SetCollisionEnabled(isVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}