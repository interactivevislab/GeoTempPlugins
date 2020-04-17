#pragma once

#include "CoreMinimal.h"
#include "ITilePreparer.h"
#include "TilesBasics.h"

#include "TilesContainer.generated.h"

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class UTileTextureContainer : public UObject
{
	GENERATED_BODY()
	
private: 


	/** Map of currently loaded and cached tiles */
	UPROPERTY()
	TMap< FTileCoordinates, UTileData*> CachedTiles;

	TMap<FString, TScriptInterface<ITilePreparer>> TextureGetters;
	
public:
	friend class UTextureDownloader;
	
	

	UFUNCTION(BlueprintCallable, Category = "Default")
	void AddTextureGetter(FString channel, TScriptInterface<ITilePreparer> getter)
	{
		TextureGetters.Add(channel, getter);
	}
	
	UFUNCTION(BlueprintCallable, Category = "Default")
	void SetTileActive(FTileCoordinates tileMeta, bool active)
	{
		auto tile = CachedTiles.Find(tileMeta);
		if (tile) {
			(*tile)->IsActive = active;
			if (!active)
			{
				(*tile)->lastAcessTime = FDateTime::Now();
			}
		}
	}

	/** Url template for tile downloading */


	UTileData* GetTileMaterial(int x, int y, int z, UMaterialInterface* mat, AActor* owner);

	/** Get uninitialized tile material for current tile and start downloading texture */
	UFUNCTION(BlueprintCallable, Category = "Default")
	UTileData* GetTileMaterial(FTileCoordinates meta, UMaterialInterface* mat, AActor* owner);

	/** Check if texture already downloaded and cached */
	UFUNCTION(BlueprintCallable, Category = "Default")
	bool IsTextureLoaded(FTileCoordinates meta);

	/** Clear all caches */
	UFUNCTION(BlueprintCallable, Category = "Default")
	void Clear();
	
private:
	UFUNCTION(BlueprintCallable, Category = "Default")
	void CacheTexture(FTileCoordinates meta, UTexture* texture, FString channel);
};