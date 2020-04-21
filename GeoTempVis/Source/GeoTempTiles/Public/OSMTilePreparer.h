#pragma once

#include "ITilePreparer.h"


#include "OSMTilePreparer.generated.h"

UCLASS(BlueprintType)
class UUrlSourceTilePreparer : public UObject, public ITilePreparer
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FTileCoordinates, UTextureDownloader*> loadingImages;
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	FString UrlString = TEXT("http://a.tile.openstreetmap.org/{0}/{1}/{2}.png");
	
	void RequestTile_Implementation(UTileData* tileInfo, UMaterialInstanceDynamic* mat, UTileTextureContainer* owner, const FString& channel) override;

	void FreeLoader(FTileCoordinates meta);	
};
