#pragma once
#include "Basics.h"
#include "BuildingsData.h"
#include "RuntimeMeshComponent.h"
#include "BuildingPart.generated.h"

class ABuildingActor;

UCLASS()
class GEOTEMPBUILDINGS_API UBuildingPartComponent : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:
	UBuildingPartComponent(const FObjectInitializer& ObjectInitializer);

public:	

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void Actualize();
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	URuntimeMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Default")
	bool UseLod;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	UMaterialInterface* WallMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	UMaterialInterface* RoofMaterial;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Outer;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Default")
	TArray<FContour> Inner;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	int Floors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float Height;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	int MinFloors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	float MinHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FBuildingDates BuildingDates;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	bool OverrideHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default")
	bool AutoUpdateMesh = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Default")
	int Id;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	FString StylePalette;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	FString TriangulationFlags;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Default")
	bool UseSmoothNormals;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FVector Center;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TMap<FString, FString> Tags;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	FLinearColor Color;	

private:
	bool _isInit = false;
	static TArray<FLinearColor> AllColors;
	static float SimplifyDistance;

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void Recalc();

	//UBuildingsLoaderBase2 * Owner;

	void Init(const FBuildingPart& inPart, TMap<FString, FString> inTags/*,UBuildingsLoaderBase2 * owner = nullptr*/);

	void ReInit();

	FBuildingPart PartData;
	ABuildingActor* Parent;


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	static void SetLodDistance(float inDist = 250000) { SimplifyDistance = inDist; }
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Default")
	virtual void CreateSimpleStructure(float inZeroHeight = 0);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void SetHeightAlpha(float inHeightAlpha);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	void ApplyCurrentTime(FDateTime inCurrentTime, bool inMustHide = false);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Default")
	static void SetAllColors(TArray<FLinearColor>& inColors);
};
