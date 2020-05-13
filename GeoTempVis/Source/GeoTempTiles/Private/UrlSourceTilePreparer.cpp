#include "UrlSourceTilePreparer.h"

void UUrlSourceTilePreparer::RequestTile_Implementation(UTileData* inTileData, UTileTextureContainer* inOwner, const FString& inChannel)
{
    
    auto url = FString::Format(*UrlString, { inTileData->Meta.Z, inTileData->Meta.X, inTileData->Meta.Y });
    UTextureDownloader* loader = NewObject<UTextureDownloader>();
    loader->TileContainer = inOwner;
    loader->TilePreparer = this;
    loader->Material = inTileData->Material;
    loader->Channel = inChannel;
    inTileData->IsLoaded.Add(inChannel, false);
    loader->StartDownloadingTile(inTileData->Meta, url);        
    loadingImages.Add(inTileData->Meta, loader);
    inTileData->CheckLoaded();
}

void UUrlSourceTilePreparer::QueueLoader(UTextureDownloader* inLoader)
{
    while (isProcessLoaders) FPlatformProcess::Sleep(0.01f);
    isProcessLoaders = true;
    readyLoaders.Add(inLoader);
    isProcessLoaders = false;
}

void UUrlSourceTilePreparer::EditorTick_Implementation(float inDeltaTime)
{
    while (isProcessLoaders) FPlatformProcess::Sleep(0.01f);
    isProcessLoaders = true;
    for (int i = 0; i < readyLoaders.Num(); i++)
    {
        if (readyLoaders[i])
        {
            readyLoaders[i]->OnTextureLoadedFromDiskMainThread();
            FreeLoader(readyLoaders[i]->TextureCoords);
        }
    }
    
    readyLoaders.Empty();
    isProcessLoaders = false;
}

void UUrlSourceTilePreparer::FreeLoader(FTileCoordinates inTileCoords)
{
    if (!loadingImages.Contains(inTileCoords))
    {
        //UE_LOG(LogTemp, Warning, TEXT("Unexpected error happened: attemp to free unexisting tile loader"));
    }    
    loadingImages.Remove(inTileCoords);
}
