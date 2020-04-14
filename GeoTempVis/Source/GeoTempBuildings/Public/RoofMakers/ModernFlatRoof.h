#pragma once
#include "BuildingUtils.h"
#include "ModernFlatRoof.generated.h"


/** Roof Generator for flat roofs with barriers on roof edge */
UCLASS()
class UModernFlatRoofMaker : public UObject, public IRoofMaker
{
public:
	GENERATED_BODY()

	/** Width of barriers on roof edges */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float BarrierWidth = 80;

	/** height of barriers on roof edges */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float BarrierHeight = 70;

	//!@{
	/** Implementation of IRoofMaker */
	virtual FBuildingMeshData GenerateRoof_Implementation(const FBuildingPart& inBuildingPart, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial) override;
	//!@}
};
