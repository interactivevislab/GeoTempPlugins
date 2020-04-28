// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "ProceduralMeshComponent.h"
#include "Blueprint/AsyncTaskDownloadImage.h"
#include "GameFramework/PlayerController.h"
#include "RuntimeMeshComponent.h"
#include "Tickable.h"
#include "TileLoader.generated.h"

class UTileTextureContainer;

class UTileData;


#pragma region TileMeta

/** Tile coordinates struct for using in maps */
USTRUCT(BlueprintType)
struct FTileCoordinates
{
	GENERATED_BODY()


public:
	/** X coordinate */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int X;

	/** Y coordinate */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int Y;

	/** Z coordinate */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int Z;

	/** Comparison operator */
	bool operator==(const FTileCoordinates& v) const
	{
		return X == v.X && Y == v.Y && Z == v.Z;
	}
};

/** Hash function for FTileCoordinates */
FORCEINLINE uint32 GetTypeHash(const FTileCoordinates& k)
{
	return (std::hash<int>()(k.X) ^ std::hash<int>()(k.Y) ^ std::hash<int>()(k.Z));
}


//namespace std
//{
//	template <>
//	struct hash<FTileCoordinates>
//	{
//		size_t operator()(const FTileCoordinates& k) const
//		{
//			// Compute individual hash values for two data members and combine them using XOR and bit shifting
//			return (hash<int>()(k.X) ^ hash<int>()(k.Y) ^ hash<int>()(k.Z));
//		}
//	};
//}
//#pragma endregion

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

	void PostLoad() override;
	
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

	/** Flag to check if files were loaded */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool AreTilesLoaded;
	
private: 

	/** free indices in tile array */
	UPROPERTY()
	TArray<int> freeIndices;

	/** Map of tile indices in array */
	UPROPERTY(NoClear)
	TMap<FTileCoordinates, int> TileIndecies;

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
	int CreateTileMesh(int x, int y, int z);

	/** create and mesh section for a tile */
	int CreateTileMesh(FTileCoordinates meta);

	/** create mesh section of a tile */	
	void ClearTileMesh(FTileCoordinates meta);

	/** check if this tile is currently split */
	bool IsTileSplit(int x, int y, int z);

	/** split tile */
	void SplitTile(FTileCoordinates meta);

	/** split tile */
	void SplitTile(int x, int y, int z);
};

/** Handle for tile web loading delegates */
UCLASS()
class UTextureDownloader : public UObject
{
	GENERATED_BODY()
public:

	/** Coordinates of the tile loading */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FTileCoordinates TextureCoords;

	/** Pointer to web loading task*/
	UPROPERTY()
	UAsyncTaskDownloadImage* Loader;

	/** Initializer for beginning of tile download */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void StartDownloadingTile(FTileCoordinates meta, FString url);

	/** Pointer to tile texture container */
	UPROPERTY()
	UTileTextureContainer* TileContainer;

	/** Pointer to material generated for the tile loading */
	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	/** Event to call when tile sucessfully loaded */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnTextureLoaded(UTexture2DDynamic* Texture);

	/** Event to call when tile download has failed */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnLoadFailed(UTexture2DDynamic* Texture);
};

/** Tile data container */
UCLASS()
class UTileData : public UObject
{
	GENERATED_BODY()
public:

	/** Pointer to tile texture container */
	UPROPERTY()
	UTileTextureContainer* Container;

	/** Is this tile currently active and visible */
	UPROPERTY()
	bool IsActive;

	/** Is this tile currently loaded and ready */
	UPROPERTY()
	bool IsLoaded;

	/** Last time this tile was requested on tile generation process */
	UPROPERTY()
	FDateTime lastAcessTime;

	/** Pointer for texture of this tile */
	UPROPERTY()
	UTexture* Texture;

	/** Pointer to material generated for the tile loading */
	UPROPERTY()
	UMaterialInstanceDynamic* Material;
};

UCLASS()
class UTileTextureContainer : public UObject
{
	GENERATED_BODY()
	
private: 
	UPROPERTY()
	TMap<FTileCoordinates, UTextureDownloader*> loadingImages;
	
public:
	friend class UTextureDownloader;
	
	/** Map of currently loaded and cached tiles */
	UPROPERTY()
	TMap< FTileCoordinates, UTileData*> CachedTiles;

	/** Url template for tile downloading */
	UPROPERTY()
	FString UrlString = TEXT("http://a.tile.openstreetmap.org/{0}/{1}/{2}.png");

	//UFUNCTION(BlueprintCallable, Category = "Default")
	UTileData* GetTileMaterial(int x, int y, int z, UMaterialInterface* mat, AActor* owner);

	/** Get uninitialized tile material for current tile and start downloading texture */
	UFUNCTION(BlueprintCallable, Category = "Default")
	UTileData* GetTileMaterial(FTileCoordinates meta, UMaterialInterface* mat, AActor* owner);

	/** Check if texture already downloaded and cached */
	UFUNCTION(BlueprintCallable, Category = "Default")
	bool IsTextureLoaded(FTileCoordinates meta);

	/** Clear all caches */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void Clear();
	
private:
	UFUNCTION(BlueprintCallable, Category = "Default")
	void CacheTexture(FTileCoordinates meta, UTexture* texture);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void FreeLoader(FTileCoordinates meta);
};

