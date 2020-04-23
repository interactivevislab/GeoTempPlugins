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

class UTileInfo;


#pragma region TileMeta

USTRUCT(BlueprintType)
struct FTileTextureMeta
{
	GENERATED_BODY()


public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int X;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int Y;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
		int Z;

	bool operator==(const FTileTextureMeta& v) const
	{
		return X == v.X && Y == v.Y && Z == v.Z;
	}
};


FORCEINLINE uint32 GetTypeHash(const FTileTextureMeta& k)
{
	return (std::hash<int>()(k.X) ^ std::hash<int>()(k.Y) ^ std::hash<int>()(k.Z));
}


namespace std
{
	template <>
	struct hash<FTileTextureMeta>
	{
		size_t operator()(const FTileTextureMeta& k) const
		{
			// Compute individual hash values for two data members and combine them using XOR and bit shifting
			return (hash<int>()(k.X) ^ hash<int>()(k.Y) ^ hash<int>()(k.Z));
		}
	};
}
#pragma endregion


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOTEMPTILES_API UTilesController : public URuntimeMeshComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTilesController(const FObjectInitializer& ObjectInitializer);

protected:

public:
	
	bool tickEnabled = false;

	void BeginPlay() override;

	void PostLoad() override;
	
	void TickComponent(float DeltaTime,enum ELevelTick TickType,FActorComponentTickFunction * ThisTickFunction) override;

	float GetPixelSize(FTileTextureMeta meta);

	UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
	void CreateMesh();

	UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
	void ClearMesh();


	UFUNCTION(BlueprintCallable, Category = "Tiles")
	void CreateMeshAroundPoint(int z, int x0, int y0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	UMaterialInterface* TileMaterial;	


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	UTileTextureContainer* TileLoader;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	float CenterLon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	float CenterLat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	TMap<FTileTextureMeta, UTileInfo*> Tiles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	APlayerController* PlayerController;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	FVector Offset;

	UPROPERTY()
	TArray<int> freeIndices;
	UPROPERTY(NoClear)
	TMap<FTileTextureMeta, int> TileIndecies;
	

	UPROPERTY()
	TSet<FTileTextureMeta> SplitTiles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int BaseLevel = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int BaseLevelSize = 8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
	int MaxLevel = 18;

	UFUNCTION(BlueprintCallable, Category = "Math")
	void GetMercatorXYFromOffset(FVector offsetValue, int z, int& x, int& y);

	UFUNCTION(BlueprintCallable, Category = "Math")
	void GetMercatorXYOffsetFromOffset(FVector offsetValue, int z, int& x, int& y);

	UFUNCTION(BlueprintCallable, Category = "Math")
	FVector GetXYOffsetFromMercatorOffset(int z, int x, int y);

private:
	static float GetMercatorXFromDegrees(double lon)
	{
		return ((lon / 180 * PI) + PI) / 2 / PI;
	}	

	static float GetMercatorYFromDegrees(double lat)
	{
		return (PI - FMath::Loge(FMath::Tan(PI / 4 + lat * PI / 180 / 2))) / 2 / PI;
	}

	
	static double EarthRadius;// = 6378.137;
	static double EarthOneDegreeLengthOnEquatorMeters;// = 111152.8928;

	int CreateTileMesh(int x, int y, int z);

	int CreateTileMesh(FTileTextureMeta meta);

	void ClearTileMesh(FTileTextureMeta meta);

	bool IsTileSplit(int x, int y, int z);

	void SplitTile(FTileTextureMeta meta);
	
	void SplitTile(int x, int y, int z);

	UPROPERTY()
	bool areTilesLoaded;
};


UCLASS()
class UTextureDownloader : public UObject
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	FTileTextureMeta TextureMeta;

	UPROPERTY()
	UAsyncTaskDownloadImage* _loader;

	UFUNCTION(BlueprintCallable, Category = "Default")
	void StartDownloadingTile(FTileTextureMeta meta, FString url);

	UPROPERTY()
	UTileTextureContainer* TileContainer;

	UPROPERTY()
	UMaterialInstanceDynamic* material;

	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnTextureLoaded(UTexture2DDynamic* Texture);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnLoadFailed(UTexture2DDynamic* Texture);
};

UCLASS()
class UTileInfo : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UTileTextureContainer* Container;

	UPROPERTY()
	bool IsActive;

	UPROPERTY()
	bool IsLoaded;
	
	UPROPERTY()
	FDateTime lastAcessTime;

	UPROPERTY()
	UTexture* Texture;

	UPROPERTY()
	UMaterialInstanceDynamic* Material;
};

UCLASS()
class UTileTextureContainer : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FTileTextureMeta, UTextureDownloader*> loadingImages;
public:
	UPROPERTY()
	TMap< FTileTextureMeta, UTileInfo*> CachedTiles;	
	
	UPROPERTY()
	FString UrlString = TEXT("http://a.tile.openstreetmap.org/{0}/{1}/{2}.png");

	//UFUNCTION(BlueprintCallable, Category = "Default")
	UTileInfo* GetTileMaterial(int x, int y, int z, UMaterialInterface* mat, AActor* owner);

	UFUNCTION(BlueprintCallable, Category = "Default")
	UTileInfo* GetTileMaterial(FTileTextureMeta meta, UMaterialInterface* mat, AActor* owner);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void CacheTexture(FTileTextureMeta meta, UTexture* texture);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void FreeLoader(FTileTextureMeta meta);

	UFUNCTION(BlueprintCallable, Category = "Default")
	bool IsTextureLoaded(FTileTextureMeta meta);

	UFUNCTION(BlueprintCallable, Category = "Default")
	void Clear();
private:
};

