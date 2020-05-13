#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "GameFramework/PlayerController.h"
#include "Tickable.h"
#include "Engine/Texture2DDynamic.h"
#include "ImageDownloadOverride.h"
#include "TilesBasics.generated.h"

UINTERFACE(MinimalAPI)
class UHeightCalculator : public UInterface
{
public:
    GENERATED_BODY()
};


/** Interface handler for different approaches to parse elevation from pixel value*/
class IHeightCalculator
{
public:
    GENERATED_BODY()    

    /** Calculate height in meters based on color value*/
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
    float CalcHeight(FColor inColor);
};


class UTileTextureContainer;
class UUrlSourceTilePreparer;
class UTileData;


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

/** Delegate for single tile texture download callbacks */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileCompleteDownloadTexture, UTileData*, Container, FString, Channel);
/** Delegate for whole tile download callbacks */
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

    /** Coordinates metadata */
    UPROPERTY()
    FTileCoordinates Meta;

    /** Callback delegate for called when finishing download of tile texture*/
    UPROPERTY(BlueprintAssignable)
    FOnTileCompleteDownloadTexture OnTextureLoad;

    /** Callback delegate for called when finishing download of tile texture with that tile and neighbors affecting heightmap*/
    UPROPERTY(BlueprintAssignable)
    FOnTileCompleteDownloadTexture OnTextureLoadWithNeighbours;

    /** Callback delegate for called when finishing download of all textures*/
    UPROPERTY(BlueprintAssignable)
    FOnTileCompleteDownloadAll OnTileLoad;

    /** Callback delegate for called when finishing download of all textures with that tile and neighbors affecting heightmap*/
    UPROPERTY(BlueprintAssignable)
    FOnTileCompleteDownloadAll OnTileLoadWithNeighbors;

    /** TileContainer responsive for loading of this tile */
    UPROPERTY()
    UTileTextureContainer* Container;

    /** Dictionary of tile loading states for neighbor right of this texture*/
    UPROPERTY()
    TMap<FString, bool> IsRightLoaded;

    /** Dictionary of tile loading states for neighbor bottom of this texture*/
    UPROPERTY()
    TMap<FString, bool> IsBottomLoaded;    

    /** Dictionary of tile loading states for neighbor bottom right of this texture*/
    UPROPERTY()
    TMap<FString, bool> IsBottomRightLoaded;

    /** Check if this tile loaded the textures and call all required callbacks */
    UFUNCTION()
    void CheckLoaded();

    /** Check if this tile loaded the textures and call all required callbacks */
    UFUNCTION()
    void CheckNeighborLoaded();

    /** Get pointer to the data of right neighbor */
    UTileData* GetRightNeighbor();
    
    /** Get pointer to the data of left neighbor */
    UTileData* GetLeftNeighbor();
    
    /** Get pointer to the data of top neighbor */
    UTileData* GetTopNeighbor();
    
    /** Get pointer to the data of bottom neighbor */
    UTileData* GetBottomNeighbor();

    /** Get pointer to the data of top left neighbor */
    UTileData* GetTopLeftNeighbor();
    
    /** Get pointer to the data of bottom right neighbor */
    UTileData* GetBottomRightNeighbor();

    /** Get pointer to the data of bottom left neighbor */
    UTileData* GetBottomLeftNeighbor();
    
    /** Get pointer to the data of top right neighbor */
    UTileData* GetTopRightNeighbor();
};