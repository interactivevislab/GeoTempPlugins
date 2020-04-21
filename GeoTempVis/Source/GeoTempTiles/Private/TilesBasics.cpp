#include "TilesBasics.h"
#include "TilesContainer.h"
#include "OSMTilePreparer.h"
#include "CoreMinimal.h"

void UTextureDownloader::StartDownloadingTile(FTileCoordinates meta, FString url)
{
	TextureCoords = meta;	
	Loader = UAsyncTaskDownloadImage::DownloadImage(url);
	Loader->OnSuccess.AddDynamic(this, &UTextureDownloader::OnTextureLoaded);
	Loader->OnFail.AddDynamic(this, &UTextureDownloader::OnLoadFailed);
}

void UTextureDownloader::OnTextureLoaded(UTexture2DDynamic* Texture)
{
	if (!Texture->IsValidLowLevel())
	{
		TilePreparer->FreeLoader(TextureCoords);
		UE_LOG(LogTemp, Warning, TEXT("Loaded texture is corrupt"));
		return;
	}
	if(!Material->IsValidLowLevel())
	{
		TilePreparer->FreeLoader(TextureCoords);
		UE_LOG(LogTemp, Warning, TEXT("Texture loaded for already destroyed tile"));
		return;
	}
	TileContainer->CacheTexture(TextureCoords, (UTexture*)(Texture), Channel);
	Material->SetTextureParameterValue(FName(*Channel), (UTexture*)(Texture));	
	
}

void UTextureDownloader::OnLoadFailed(UTexture2DDynamic* Texture)
{
	TilePreparer->FreeLoader(TextureCoords);
	UE_LOG(LogTemp, Warning, TEXT("Load failed"));
}

