#include "TilesBasics.h"


#include "TilesContainer.h"
#include "OSMTilePreparer.h"
#include "Materials/MaterialInstanceDynamic.h"

void UTextureDownloader::StartDownloadingTile(FTileCoordinates meta, FString url)
{
	TextureCoords = meta;	
	Loader = UImageDownloadOverride::DownloadImage(url);
	Loader->OnSuccess.AddDynamic(this, &UTextureDownloader::OnTextureLoaded);
	Loader->OnFail.AddDynamic(this, &UTextureDownloader::OnLoadFailed);
}

void UTextureDownloader::OnTextureLoaded(UTexture2DDynamic* Texture, TArray<uint8> data)
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
	Material->SetTextureParameterValue(FName(*Channel), Texture);	
	TileContainer->CacheTexture(TextureCoords, Texture, Channel, data);
	
	
}

void UTextureDownloader::OnLoadFailed(UTexture2DDynamic* Texture, TArray<uint8> data)
{
	TilePreparer->FreeLoader(TextureCoords);
	UE_LOG(LogTemp, Warning, TEXT("Load failed"));
}

void UTileData::CheckLoaded()
{
	bool allLoaded = true;

	auto left		= GetLeftNeighbor();
	auto top		= GetTopNeighbor();
	auto topLeft	= GetTopLeftNeighbor();	
	for (auto& kv : IsLoaded)
	{
		allLoaded &= kv.Value;
		if (kv.Value) OnTextureLoad.Broadcast(this, kv.Key);
		if (left) 
		{
			left->		IsRightLoaded.Add(kv.Key, kv.Value);
		}
		if (top) 
		{
			top->		IsBottomLoaded.Add(kv.Key, kv.Value);
		}
		if (topLeft)
		{
			topLeft->	IsBottomRightLoaded.Add(kv.Key, kv.Value);
		}
	}
	if (allLoaded) OnTileLoad.Broadcast(this);
	if (left) left->		CheckNeighborLoaded();
	if (top) top->		CheckNeighborLoaded();
	if (topLeft) topLeft->	CheckNeighborLoaded();
	
	
}

void UTileData::CheckNeighborLoaded()
{
	bool allLoaded = true;
	for (auto& kv : IsLoaded)
	{
		auto loaded = kv.Value && IsRightLoaded.FindRef(kv.Key) && IsBottomLoaded.FindRef(kv.Key) && IsBottomRightLoaded
			.FindRef(kv.Key);
		allLoaded &= (loaded);
		if (loaded) OnTextureLoadWithNeighbours.Broadcast(this, kv.Key);
	}
	if (allLoaded) OnTileLoadWithNeighbors.Broadcast(this);
}

UTileData* UTileData::GetRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y    , Meta.Z});
}

UTileData* UTileData::GetLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y    , Meta.Z});
}

UTileData* UTileData::GetTopNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X    , Meta.Y - 1, Meta.Z});
}

UTileData* UTileData::GetBottomNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X    , Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetTopLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y - 1, Meta.Z});
}

UTileData* UTileData::GetBottomRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetBottomLeftNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X - 1, Meta.Y + 1, Meta.Z});
}

UTileData* UTileData::GetTopRightNeighbor()
{
	return Container->CachedTiles.FindRef(FTileCoordinates{Meta.X + 1, Meta.Y - 1, Meta.Z});
}


