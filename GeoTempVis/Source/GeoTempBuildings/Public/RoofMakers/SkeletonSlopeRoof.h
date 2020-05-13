#pragma once
#include "BuildingUtils.h"
#include "SkeletonSlopeRoof.generated.h"

/** Roof Generator for slope roofs based on preset crest skeleton stored in Building Part structure */
UCLASS()
class USlopeRoofMaker : public UObject, public IRoofMaker
{
public:
    GENERATED_BODY()

    /** Height of roof crest over the walls top point */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    float RoofHeight;

    /** @name Implementation of IRoofMaker */
    ///@{
    virtual FBuildingMeshData GenerateRoof_Implementation(const FBuildingPart& inBuildingPart, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial) override;
    ///@}
};
