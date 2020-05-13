#pragma once

#include "EditorTickable.h"
#include "ITileProvider.h"
#include "TextureDownloader.h"

#include "UrlSourceTilePreparer.generated.h"


UCLASS(BlueprintType)
class UUrlSourceTilePreparer : public UObject, public ITileProvider, public IEditorTickable
{
    GENERATED_BODY()

    /** List of active texture downloaders */
    UPROPERTY()
    TMap<FTileCoordinates, UTextureDownloader*> loadingImages;
    
    /** List of downloaders ready to process in main thread */
    TArray<UTextureDownloader*> readyLoaders;

    bool isProcessLoaders = false;
public:

    /** Url template for tile downloading. Uses arguments in order of z, x, y */
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Meta=(ExposeOnSpawn="true"), Category= "Default")
    FString UrlString = TEXT("http://a.tile.openstreetmap.org/{0}/{1}/{2}.png");

    /** @name Implementation of IModuleInterface */
    ///@{
    void RequestTile_Implementation(UTileData* inTileData, UTileTextureContainer* inOwner, const FString& inChannel) override;
    ///@}

    /** add loader to queue for main thread processing */
    void QueueLoader(UTextureDownloader* inLoader);
    
    /** @name Implementation of IEditorTickable */
    ///@{
    void EditorTick_Implementation(float inDeltaTime) override;
    ///@}

    /** Remove texture loader for current tile coordinates from the list */
    void FreeLoader(FTileCoordinates inTileCoords);
};
