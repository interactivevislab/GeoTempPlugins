#include "TilesContainer.h"
#include "Engine/Engine.h"
#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"

UTileData* UTileTextureContainer::GetTileMaterial(int x, int y, int z, UMaterialInterface* mat, AActor* owner)
{
	auto meta = FTileCoordinates{ x, y, z };
	return GetTileMaterial(meta, mat, owner);
}

UTileData* UTileTextureContainer::GetTileMaterial(FTileCoordinates meta, UMaterialInterface* mat, AActor* owner)
{
	//UE_LOG(LogTemp, Warning, TEXT("Total cached textures: %i"), CachedTiles.Num());
	auto cached = CachedTiles.Find(meta);
	if (!cached || !*cached)
	{
		if (CachedTiles.Num() > 512)
		{
			TArray<FTileCoordinates> pendingDelete;		
			for (auto cachedTile : CachedTiles)
			{
				if (!cachedTile.Value || !cachedTile.Value->IsActive && (cachedTile.Value->lastAcessTime - FDateTime::Now()).GetTotalSeconds() > 60)
				{
					pendingDelete.Add(cachedTile.Key);
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
		
		UMaterialInstanceDynamic* matInstance = UMaterialInstanceDynamic::Create(mat, owner);
		
		auto TileInfo = NewObject<UTileData>();
		TileInfo->Meta = meta;
		TileInfo->Material = matInstance;
		TileInfo->lastAcessTime = FDateTime::Now();
		TileInfo->Container = this;

		
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


void UTileTextureContainer::CacheTexture(FTileCoordinates meta, UTexture* texture, FString channel, TArray<uint8> data)
{
	if (CachedTiles.Contains(meta))
	{
		if (texture && texture->IsValidLowLevel())
		{
			CachedTiles[meta]->Textures.Add(channel, texture);
			CachedTiles[meta]->IsLoaded[channel] = true;
			
		}
		if (channel.Equals(ElevationChannel))
		{
			auto& heightmap = CachedTiles[meta]->HeightMap;
			heightmap.Empty();
			for (int i = 0; i < data.Num() / 4; i++)
			{
				heightmap.Add(FColor(data[i * 4 + 2], data[i * 4 + 1], data[i * 4], data[i * 4 + 3]));
			}
			CachedTiles[meta]->HeightMap = heightmap;
		}
		CachedTiles[meta]->CheckLoaded();
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
