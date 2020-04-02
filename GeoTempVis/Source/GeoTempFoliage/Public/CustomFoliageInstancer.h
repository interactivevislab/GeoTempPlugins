// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Math/Vector.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FoliageInfo.h"
#include "Basics.h"
#include "MasksLoader.h"
#include "BasePolygonPreparer.h"
#include "CustomFoliageInstancer.generated.h"


UENUM(BlueprintType)		
enum class ELayersOption : uint8
{
	MonoLayered		UMETA(DisplayName = "MonoLayered"),
	PolyLayered		UMETA(DisplayName = "PolyLayered"),
};

UCLASS(BlueprintType)
class GEOTEMPFOLIAGE_API ACustomFoliageInstancer : public AActor
{
	GENERATED_BODY()

public:
	ACustomFoliageInstancer();

private:
	TArray<float> maskBuffer;
	TArray<FColor> colorBuffer;

	bool updateSecondRenderTarget;
	int currentMaskIndex;
	TArray<int> polygonDates;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;


	UPROPERTY()
	USceneComponent* Root;

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
	UTexture2D* InitialMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
	UTexture2D* BlankMask;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
	TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> FoliageInstancers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
	TArray<FPosgisContourData> InputPolygons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
	ProjectionType Projection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
	float OriginLon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
	float OriginLat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool DrawGizmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (UIMin = "0.0", UIMax = "1.0"))
	float CurrentInterpolation;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigurationSettings")
	UMaskLoader* MaskLoaderInitial;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigurationSettings")
	UMaskLoader* MaskLoaderFirst;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigurationSettings")
	UMaskLoader* MaskLoaderSecond;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigurationSettings")
	UMaskLoader* MaskLoaderTypes;

	UPROPERTY(BlueprintReadWrite, Category = "ConfigurationSettings")
	UBasePolygonPreparer* PolygonPreparer;

	UFUNCTION(BlueprintCallable)
	void FillFoliage_BP(
		FVector4 componentRect, 
		bool DebugDraw = false, 
		bool updateMaskBuffer = false
	);

	UFUNCTION()
	void FillFoliage(
		TArray<FFoliageMeshInfo> Infos, 
		TArray<UHierarchicalInstancedStaticMeshComponent*> Instancers, 
		bool DebugDraw = false, 
		bool updateMaskBuffer = false
	);
	
	UFUNCTION(CallInEditor, Category = "Default")
	void BufferMask(UTexture2D* inInitialMask);
	
	UFUNCTION()
	void InterpolateFoliageWithMaterial(FFoliageMeshInfo Fol);
	
	UFUNCTION(BlueprintCallable, Category = "Default")
	void UpdateBuffer();
	
	UFUNCTION(BlueprintCallable, Category = "Default")
	FColor GetRenderTargetValue(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void UpdateFoliageMasks(FDateTime currentTime);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void GetDatesNearCurrent(FDateTime currentTime);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void ParseDates();

	UFUNCTION(BlueprintCallable, Category = "Default")
	void ParseTimeTags(FPosgisContourData contour, TSet<int>& outDates);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void SortDatesByAscend(TSet<int> dates);
};
