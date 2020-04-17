#include "TilesContainer.h"

#include "Engine/Engine.h"

UTileData* UTileTextureContainer::GetTileMaterial(int x, int y, int z, UMaterialInterface* mat, AActor* owner)
{
	auto meta = FTileCoordinates{ x, y, z };
	return GetTileMaterial(meta, mat, owner);
}

UTileData* UTileTextureContainer::GetTileMaterial(FTileCoordinates meta, UMaterialInterface* mat, AActor* owner)
{
	//UE_LOG(LogTemp, Warning, TEXT("Total cached textures: %i"), CachedTiles.Num());

	if (CachedTiles.Num() > 512)
	{
		TArray<FTileCoordinates> pendingDelete;		
		for (auto cached : CachedTiles)
		{
			//if (!cached.Value)
			//{
			//	//all gone wrong. Probably we somehow lost all cache on engine reload or something like that
			//	CachedTiles.Empty();
			//	break;
			//}
			if (!cached.Value || !cached.Value->IsActive && (cached.Value->lastAcessTime - FDateTime::Now()).GetTotalSeconds() > 60)
			{
				pendingDelete.Add(cached.Key);
			}
		}
		for (auto tile : pendingDelete)
		{
			auto t = CachedTiles[tile];
			if (t) {
				t->IsLoaded.Empty();
			}
			CachedTiles.Remove(tile);			 
		}
	}
	auto cached = CachedTiles.Find(meta);
	if (!cached || !*cached)
	{
		UMaterialInstanceDynamic* matInstance = UMaterialInstanceDynamic::Create(mat, owner);
		
		auto TileInfo = NewObject<UTileData>();
		TileInfo->Meta = meta;
		TileInfo->Material = matInstance;
		TileInfo->lastAcessTime = FDateTime::Now();

		
		CachedTiles.Add(meta, TileInfo);
	
		for (auto textureGetter : TextureGetters)
		{
			auto getter = textureGetter.Value;
			const auto& interface = Cast<ITilePreparer>(getter.GetObject());
			interface->Execute_RequestTile(getter.GetObject(), TileInfo, matInstance, this, textureGetter.Key);
		}
		//matInstance->SetTextureParameterValue("Tile", CachedTextures[meta]);		
		return TileInfo;
	}	
	else
	{		
		return *cached;
	}
}


void UTileTextureContainer::CacheTexture(FTileCoordinates meta, UTexture* texture, FString channel)
{
	if (CachedTiles.Contains(meta))
	{
		if (texture && texture->IsValidLowLevel())
		{
			CachedTiles[meta]->Textures.Add(channel, texture);
			CachedTiles[meta]->IsLoaded[channel] = true;
		}
		
		
	}
	
}

bool UTileTextureContainer::IsTextureLoaded(FTileCoordinates meta)
{
	if (CachedTiles.Contains(meta)) return false;
	for (auto x : CachedTiles[meta]->IsLoaded)
	{
		if (!x.Value) return false;
	}
	return true;
}

void UTileTextureContainer::Clear()
{
	CachedTiles.Empty();
}
