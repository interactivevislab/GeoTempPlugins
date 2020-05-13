#pragma once

#include "CoreMinimal.h"
#include "TextureResource.h"
#include "GameFramework/PlayerController.h"
#include "Tickable.h"
#include "Engine/Texture2DDynamic.h"
#include "ImageDownloadOverride.h"
#include "TilesContainer.h"
#include "Async/AsyncFileHandle.h"
#include "TextureDownloader.generated.h"


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
    void StartDownloadingTile(FTileCoordinates inTileCoords, const FString& inUrl, FString inCacheFolder = "/Cache", FString inCacheFile = "");

    /** Pointer to tile texture container */
    UPROPERTY()
    UTileTextureContainer* TileContainer;

    /** Url tile source that requested this downloader*/
    UPROPERTY()
    UUrlSourceTilePreparer* TilePreparer;
    
    /** Pointer to material generated for the tile loading */
    UPROPERTY()
    UMaterialInstanceDynamic* Material;

    /** Texture channel to save to */
    UPROPERTY()
    FString Channel;

    /** Path to cache folder */
    UPROPERTY()
    FString CachePath;

    /** Name of cache file*/
    UPROPERTY()
    FString CacheFile;

    /** Resolution of loaded texture */
    UPROPERTY()
    int Resolution = 256;

    /** Check if cache file is already exists */
    UFUNCTION()
    bool CheckCache();

    /** Load data from existing cache file */
    UFUNCTION()
    void LoadCache();

    /** Save cache into */
    UFUNCTION()
    void WriteCache();
    
    /** Event to call when tile sucessfully loaded */
    UFUNCTION()
    void OnTextureLoadedWeb(UTexture2DDynamic* inTexture, TArray<uint8> inData);

    /** Event to call when tile sucessfully loaded */
    UFUNCTION()
    void OnTextureLoadedFromDisk();
    
    void OnTextureLoadedFromDiskMainThread();

    /** Event to call when tile download has failed */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void OnLoadFailed(UTexture2DDynamic* inTexture, TArray<uint8> inData);


private:
    /** pixels of a texture stored in byte array */
    TArray<uint8> textureInMemory;
    FAsyncFileCallBack ReadCallbackFunction;
};
