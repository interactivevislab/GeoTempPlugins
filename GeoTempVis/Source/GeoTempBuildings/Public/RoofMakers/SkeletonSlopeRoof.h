#pragma once
#include "BuildingUtils.h"
#include "SkeletonSlopeRoof.generated.h"

UCLASS()
class USlopeRoofMaker : public UObject, public IRoofMaker
{
public:
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	float RoofHeight;
	
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Default")
	//FBuildingMeshData GenerateRoof(FBuildingPart buildingPart, int firstSectionIndex, UMaterialInterface* wallMaterial, UMaterialInterface* roofMaterial) ;

	virtual FBuildingMeshData GenerateRoof_Implementation(FBuildingPart inBuildingPart, int inFirstSectionIndex, UMaterialInterface* inWallMaterial, UMaterialInterface* inRoofMaterial) override;
};
