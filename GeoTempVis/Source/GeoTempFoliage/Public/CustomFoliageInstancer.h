// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FoliageInfo.h"
#include "GeometryData.h"
#include "CustomFoliageInstancer.generated.h"


UENUM(BlueprintType)		
enum class ELayersOption : uint8
{
	MonoLayered		UMETA(DisplayName = "MonoLayered"),
	PolyLayered		UMETA(DisplayName = "PolyLayered"),
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class GEOTEMPFOLIAGE_API UCustomFoliageInstancer : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomFoliageInstancer();

private:
	TArray<float> maskBuffer;
	TArray<FColor> colorBuffer;

	bool updateSecondRenderTarget;
	int currentMaskIndex;
	TArray<int> polygonDates;

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	FVector2D CellSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings", meta = (UIMin = "0.0", UIMax = "360.0"))
	float SpawnAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	FVector2D SpawnTranslate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	float InstancerSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	ELayersOption MeshLayersOption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	UTextureRenderTarget2D* StartTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	UTextureRenderTarget2D* EndTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	UTextureRenderTarget2D* TypesTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	UTextureRenderTarget2D* InitialTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
	TArray<FFoliageMeshInfo> FoliageMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
	TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> FoliageInstancers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
	TMap<FString, FString> DataBaseTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool DrawGizmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (UIMin = "0.0", UIMax = "1.0"))
	float CurrentInterpolation;

	UFUNCTION(BlueprintCallable)
	void FillFoliage_BP(
		FVector4 inComponentRect, 
		bool inUpdateMaskBuffer = false
	);

	UFUNCTION(BlueprintCallable)
	void ClearFoliage_BP();

	UFUNCTION()
	void FillFoliageWithMeshes(
		TArray<FFoliageMeshInfo>& inInfos, 
		TArray<UHierarchicalInstancedStaticMeshComponent*>& inInstancers, 
		bool inUpdateMaskBuffer = false
	);
	
	UFUNCTION(BlueprintCallable)
	void InterpolateFoliageWithMaterial();
	
	UFUNCTION(BlueprintCallable, Category = "Default")
	void UpdateBuffer();

	UFUNCTION(BlueprintCallable, Category = "Default")
	void UpdateFoliageMasksDates(FDateTime inCurrentTime, int& outRenderYearFirst, int& outRenderYearSecond, bool& outUpdateFirstTarget);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void GetDatesNearCurrent(FDateTime inCurrentTime);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void ParseDates(TArray<FContourData>& inContours);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void ParseTimeTags(FContourData inContour, TSet<int>& outDates);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void SortDatesByAscend(TSet<int>& inDates);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void FillFoliage_BP_Test(float inComponentRect, UStaticMesh* inMesh, UHierarchicalInstancedStaticMeshComponent* outComponent);
};
