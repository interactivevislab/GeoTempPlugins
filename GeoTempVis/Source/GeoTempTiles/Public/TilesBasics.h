#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "GameFramework/PlayerController.h"
#include "Tickable.h"
#include "Engine/Texture2DDynamic.h"
#include "ImageDownloadOverride.h"
#include "TilesBasics.generated.h"


class UTileTextureContainer;
class UUrlSourceTilePreparer;
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
	UImageDownloadOverride* Loader;

	/** Initializer for beginning of tile download */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void StartDownloadingTile(FTileCoordinates meta, FString url);

	/** Pointer to tile texture container */
	UPROPERTY()
	UTileTextureContainer* TileContainer;

	UPROPERTY()
	UUrlSourceTilePreparer* TilePreparer;
	
	/** Pointer to material generated for the tile loading */
	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY()
	FString Channel;
	
	/** Event to call when tile sucessfully loaded */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnTextureLoaded(UTexture2DDynamic* Texture, TArray<uint8> data);

	/** Event to call when tile download has failed */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void OnLoadFailed(UTexture2DDynamic* Texture, TArray<uint8> data);
};


class UTileData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileCompleteDownloadTexture, UTileData*, Container, FString, Channel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileCompleteDownloadAll, UTileData*, Container);

/** Tile data container */
UCLASS()
class UTileData : public UObject
{
	GENERATED_BODY()
public:

	/** Is this tile currently active and visible */
	UPROPERTY()
	bool IsActive;

	/** Is this tile currently loaded and ready */
	UPROPERTY()
	TMap<FString, bool> IsLoaded;

	/** Last time this tile was requested on tile generation process */
	UPROPERTY()
	FDateTime lastAcessTime;

	/** Pointer for texture of this tile */
	UPROPERTY()
	TMap<FString, UTexture*> Textures;

	UPROPERTY()
	TArray<FColor> HeightMap;
	
	/** Pointer to material generated for the tile loading */
	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY()
	FTileCoordinates Meta;

	UPROPERTY(BlueprintAssignable)
	FOnTileCompleteDownloadTexture OnTextureLoad;

	UPROPERTY(BlueprintAssignable)
	FOnTileCompleteDownloadTexture OnTextureLoadWithNeighbours;

	UPROPERTY(BlueprintAssignable)
	FOnTileCompleteDownloadAll OnTileLoad;
	
	UPROPERTY(BlueprintAssignable)
	FOnTileCompleteDownloadAll OnTileLoadWithNeighbors;

	UPROPERTY()
	UTileTextureContainer* Container;
	
	UPROPERTY()
	TMap<FString, bool> IsRightLoaded;
	
	UPROPERTY()
	TMap<FString, bool> IsBottomLoaded;	
	
	UPROPERTY()
	TMap<FString, bool> IsBottomRightLoaded;

	UFUNCTION()
	void CheckLoaded();

	UFUNCTION()
	void CheckNeighborLoaded();

	UTileData* GetRightNeighbor();
	UTileData* GetLeftNeighbor();
	UTileData* GetTopNeighbor();
	UTileData* GetBottomNeighbor();

	UTileData* GetTopLeftNeighbor();
	UTileData* GetBottomRightNeighbor();

	UTileData* GetBottomLeftNeighbor();
	UTileData* GetTopRightNeighbor();
};