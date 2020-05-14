#include "TilesContainer.h"
#include "Engine/Engine.h"
#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"

void UTileTextureContainer::AddTextureGetter(FString inChannel, TScriptInterface<ITileProvider> inGetter)
{
    TextureGetters.Add(inChannel, inGetter);
}

void UTileTextureContainer::SetTileActive(FTileCoordinates inTileCoords, bool inActive)
{
    auto tile = CachedTiles.Find(inTileCoords);
    if (tile)
    {
        (*tile)->IsActive = inActive;
        if (!inActive)
        {
            (*tile)->lastAcessTime = FDateTime::Now();
        }
    }
}

UTileData* UTileTextureContainer::GetTileMaterial(FTileCoordinates inTileCoords, UMaterialInterface* inMat, AActor* inOwner)
{
    //UE_LOG(LogTemp, Warning, TEXT("Total cached textures: %i"), CachedTiles.Num());
    auto cached = CachedTiles.Find(inTileCoords);
    if (!cached || !*cached || (*cached)->IsLoaded.Num() == 0)
    {
        if (CachedTiles.Num() > 128)
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
        
        UMaterialInstanceDynamic* matInstance = UMaterialInstanceDynamic::Create(inMat, inOwner);
        
        auto TileInfo = NewObject<UTileData>();
        TileInfo->Meta = inTileCoords;
        TileInfo->Material = matInstance;
        TileInfo->lastAcessTime = FDateTime::Now();
        TileInfo->Container = this;

        
        CachedTiles.Add(inTileCoords, TileInfo);
    
        for (auto textureGetter : TextureGetters)
        {
            auto getter = textureGetter.Value;
            const auto& interface = Cast<ITileProvider>(getter.GetObject());
            if (IsValid(getter.GetObject())) 
                interface->Execute_RequestTile(getter.GetObject(), TileInfo, this, textureGetter.Key);
        }
        //matInstance->SetTextureParameterValue("Tile", CachedTextures[inTileCoords]);        
        return TileInfo;
    }    
    else
    {        
        return *cached;
    }
}


void UTileTextureContainer::CacheTexture(FTileCoordinates inTileCoords, UTexture* inTexture, FString inChannel, const TArray<uint8>& inData)
{
    if (CachedTiles.Contains(inTileCoords))
    {
        if (inTexture && inTexture->IsValidLowLevel())
        {
            CachedTiles[inTileCoords]->Textures.Add(inChannel, inTexture);
            CachedTiles[inTileCoords]->IsLoaded[inChannel] = true;
            
        }
        if (inChannel.Equals(ElevationChannel))
        {
            auto& heightmap = CachedTiles[inTileCoords]->HeightMap;
            heightmap.Empty();
            for (int i = 0; i < inData.Num() / 4; i++)
            {
                heightmap.Add(FColor(inData[i * 4 + 2], inData[i * 4 + 1], inData[i * 4], inData[i * 4 + 3]));
            }
            CachedTiles[inTileCoords]->HeightMap = heightmap;
        }
        CachedTiles[inTileCoords]->CheckLoaded();
    }
    
}

bool UTileTextureContainer::IsTextureLoaded(FTileCoordinates inTileCoords)
{
    if (CachedTiles.Contains(inTileCoords)) return false;
    for (auto x : CachedTiles[inTileCoords]->IsLoaded)
    {
        if (!x.Value) return false;
    }
    return true;
}

void UTileTextureContainer::Clear()
{
    CachedTiles.Empty();
}

void UTileTextureContainer::CleanMess()
{
    TArray<FTileCoordinates> mess;
	for (auto& kv : CachedTiles)
	{
        if (!kv.Value) mess.Add(kv.Key);
	}
	for (auto& key : mess)
	{
        CachedTiles.Remove(key);
	}
}
