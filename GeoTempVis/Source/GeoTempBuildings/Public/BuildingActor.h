#pragma once
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "BuildingsData.h"
#include "BuildingActor.generated.h"



class UBuildingPartComponent;

UCLASS()
class GEOTEMPBUILDINGS_API ABuildingActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildingActor();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Outer;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Inner;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Tags")
	TMap<FString, FString> BuildingTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	UMaterialInterface* WallMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	UMaterialInterface* RoofMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FString RoofType;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	int Id;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TArray<UBuildingPartComponent*> Parts;
 
	UPROPERTY()
	FBuilding Building;

	void OnConstruction(const FTransform& Transform) override;


	virtual void Initialize(const FBuilding& inBuilding, bool inInitPartsImmideately = true);

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void ReInitialize();
};
