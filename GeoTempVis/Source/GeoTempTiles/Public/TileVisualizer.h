// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "ProceduralMeshComponent.h"
#include "Blueprint/AsyncTaskDownloadImage.h"
#include "GameFramework/PlayerController.h"
#include "RuntimeMeshComponent.h"
#include "Tickable.h"
#include "TileGeometryGenerator.h"
#include "TilesBasics.h"
#include "TileVisualizer.generated.h"

class UTileTextureContainer;

class UTileData;


#pragma region TileMeta

/** Actor component for tiles handling and visualization*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOTEMPTILES_API UTilesController : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTilesController(const FObjectInitializer& ObjectInitializer);

protected:

public:

	//!@{
	/** Implementation of UActorComponent default functions */
	bool tickEnabled = false;
	
	void BeginPlay() override;
	
	void TickComponent(float DeltaTime,enum ELevelTick TickType,FActorComponentTickFunction * ThisTickFunction) override;
	//!@}

	/** Calculate tile screen size in pixels */
	float GetPixelSize(FTileCoordinates meta);

	/** Geodetic longitude of actor origin */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	float CenterLon;

	/** Geodetic latitude of actor origin */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	float CenterLat;

	/** Create tiles around central point */
	UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
	void CreateMesh();

	/** Clear all tile meshes */
	UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
	void ClearMesh();

	/** Create tiles around specific point in scene space */
	UFUNCTION(BlueprintCallable, Category = "Tiles")
	void CreateMeshAroundPoint(int z, int x0, int y0);

	/** Pointer to tiles material. Material should contain texture parameter called `Tile` */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	UMaterialInterface* TileMaterial;	

	/** Pointer to Tile Texture Container. Inits internally.
	 * @see UTileTextureContainer
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	UTileTextureContainer* TileLoader;
	
	/** Array of all Tiles currently loaded*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	TMap<FTileCoordinates, UTileData*> Tiles;

	/** Default zoom level for tiles generation */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int BaseLevel = 10;

	/** Number of base level  tiles generated along single axis */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int BaseLevelSize = 8;

	/** Max zoom level tiles will split to */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int MaxLevel = 18;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int TileMeshResolution = 4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	UTileGeometryGenerator* GeometryGenerator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	FString ElevationChannel;
	
private: 

	/** free indices in tile array */
	UPROPERTY()
	TArray<int> freeIndices;

	/** Map of tile indices in array */
	UPROPERTY(NoClear)
	TMap<FTileCoordinates, int> TileIndecies;

		UPROPERTY(NoClear)
	TMap<FTileCoordinates, int> ReservedIndecies;

	/** Tiles which are not currently present, but preserved in memory a them are parents of other tiles */
	UPROPERTY()
	TSet<FTileCoordinates> SplitTiles;

	/** Get tile coordinates based on offset vector in scene space*/
	UFUNCTION(BlueprintCallable, Category = "Math")
	void GetMercatorXYFromOffset(FVector offsetValue, int z, int& x, int& y);

	/** Get tile coordinates offset from default tile coordinates based on offset vector in scene space*/
	UFUNCTION(BlueprintCallable, Category = "Math")
	void GetMercatorXYOffsetFromOffset(FVector offsetValue, int z, int& x, int& y);

	/** Get scene coordinates of tile based on it tile coordinates */
	UFUNCTION(BlueprintCallable, Category = "Math")
	FVector GetXYOffsetFromMercatorOffset(int z, int x, int y);

	/** Calculate mercator x position based on lontitude */
	static float GetMercatorXFromDegrees(double lon)
	{
		return ((lon / 180 * PI) + PI) / 2 / PI;
	}	

	/** Calculate mercator x position based on latitude */
	static float GetMercatorYFromDegrees(double lat)
	{
		return (PI - FMath::Loge(FMath::Tan(PI / 4 + lat * PI / 180 / 2))) / 2 / PI;
	}

	/** Const earth radius */
	static double EarthRadius;// = 6378.137;
	/** Const one degree on equator length */
	static double EarthOneDegreeLengthOnEquator;// = 111152.8928;

	


	/** create and mesh section for a tile */
	UFUNCTION()
	void CreateTileMesh(UTileData* tile);
	
	
	
	/** create and mesh section for a tile */
	int BeginCreateTileMesh(int x, int y, int z);
	
	/** create and mesh section for a tile */
	int BeginCreateTileMesh(FTileCoordinates meta);

	/** create mesh section of a tile */	
	void ClearTileMesh(FTileCoordinates meta);

	/** check if this tile is currently split */
	bool IsTileSplit(int x, int y, int z);

	/** split tile */
	void SplitTile(FTileCoordinates meta);

	/** split tile */
	void SplitTile(int x, int y, int z);
};



