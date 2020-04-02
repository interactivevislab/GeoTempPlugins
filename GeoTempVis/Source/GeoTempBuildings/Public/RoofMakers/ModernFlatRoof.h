#pragma once
#include "BuildingUtils.h"
#include "ModernFlatRoof.generated.h"


UCLASS()
class UModernFlatRoofMaker : public UObject, public IRoofMaker
{
public:
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
	FBuildingMeshData GenerateRoof(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial);
	
	virtual FBuildingMeshData GenerateRoof_Implementation(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial) override;
};