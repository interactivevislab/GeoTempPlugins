#pragma once
#include "BuildingUtils.h"
#include "ModernFlatRoof.generated.h"


UCLASS()
class UModernFlatRoofMaker : public UObject, public IRoofMaker
{
public:
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float BarrierWidth = 80;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float BarrierHeight = 70;
	
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
	//FBuildingMeshData GenerateRoof(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial);
	
	virtual FBuildingMeshData GenerateRoof_Implementation(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial) override;
};