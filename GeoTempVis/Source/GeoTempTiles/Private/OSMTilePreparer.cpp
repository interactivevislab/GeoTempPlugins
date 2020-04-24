#include "OSMTilePreparer.h"

void UUrlSourceTilePreparer::RequestTile_Implementation(UTileData* tileInfo, UMaterialInstanceDynamic* mat, UTileTextureContainer* owner, const FString& channel)
{
	
	auto url = FString::Format(*UrlString, { tileInfo->Meta.Z, tileInfo->Meta.X, tileInfo->Meta.Y });
	UTextureDownloader* loader = NewObject<UTextureDownloader>();
	loader->TileContainer = owner;
	loader->TilePreparer = this;
	loader->Material = mat;
	loader->Channel = channel;
	loader->StartDownloadingTile(tileInfo->Meta, url);	
	tileInfo->IsLoaded.Add(channel, false);
	loadingImages.Add(tileInfo->Meta, loader);
	tileInfo->CheckLoaded();
}

void UUrlSourceTilePreparer::FreeLoader(FTileCoordinates meta)
{
	if (!loadingImages.Contains(meta))
	{
		UE_LOG(LogTemp, Warning, TEXT("Unexpected error happened: attemp to free unexisting tile loader"));
	}	
	loadingImages.Remove(meta);
}
