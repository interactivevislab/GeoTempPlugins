#pragma once

#include "CoreMinimal.h"
#include "TilesBasics.h"
#include "ITileProvider.generated.h"


UINTERFACE(MinimalAPI)
class UTileProvider : public UInterface
{
public:
    GENERATED_BODY()
};

/** Interface of tile providers, which loads textures for tiles */
class ITileProvider  
{ 
public:
    GENERATED_BODY()

    /** Send request for loading texture and write it into tile data object
     * @param inTileInfo TileData object of tile to load texture for
     * @param inOwner TileTextureContainer responsive for caching and requesting tile textures
     * @param inChannel channel to write texture to
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
    void RequestTile(UTileData* inTileInfo, UTileTextureContainer* inOwner, const FString& inChannel);
};


