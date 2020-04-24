#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TilesBasics.h"
#include "TileGeometryGenerator.generated.h"



UINTERFACE(MinimalAPI)
class UHeightCalculator : public UInterface
{
public:
	GENERATED_BODY()
};

class IHeightCalculator
{
public:
	GENERATED_BODY()	

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Default")
	float CalcHeight(FColor color);
};







/** Actor component for tiles handling and visualization*/
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOTEMPTILES_API UTileGeometryGenerator : public UObject
{
	GENERATED_BODY()

public:
	/** Height calculator to determine height based on texure value */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	TScriptInterface<IHeightCalculator> HeightCalculator;




	/** Path to where store cached tiles for further load*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	FString CachePath = "DefaultCache";

	/** Name of cache file for tile. Used with format(CacheNameFormat, x, y, z) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	FString CacheNameFormat = "z{2}x{0}y{1}.tile";	
	
	void GenerateVertices(UTileData* tile, FString channel, int resolution, TArray<FVector>& outVerticesWithoutElevation, TArray<FVector>& outNormals)
	{
		auto right = tile->GetRightNeighbor();
		auto top = tile->GetBottomNeighbor();
		auto topright = tile->GetBottomRightNeighbor();
		
		auto tex			= tile		->Textures.FindRef(channel);
		
		auto texFix = Cast<UTexture2DDynamic>(tex);

		//DEBUG: step by step try of loading texture to check if everything is correct. Delete that after it works
		auto texData = texFix->GetRunningPlatformData();
		if (!texData) return;
		auto& mip = (*texData)->Mips[0];
		auto w = mip.SizeX;
		auto h = mip.SizeY;


		//Actual way I plan to load the textures in one string
		auto rightTex		= right		->Textures.FindRef(channel);
		auto topTex	= top	->Textures.FindRef(channel);
		auto trTex		= topright->Textures.FindRef(channel);
		
		auto tileData	= static_cast<const FColor*>((*Cast<UTexture2DDynamic>(tex)			->GetRunningPlatformData())->Mips[0].BulkData.LockReadOnly());
		auto rightData	= static_cast<const FColor*>((*Cast<UTexture2DDynamic>(rightTex)	->GetRunningPlatformData())->Mips[0].BulkData.LockReadOnly());
		auto topData	= static_cast<const FColor*>((*Cast<UTexture2DDynamic>(topTex)		->GetRunningPlatformData())->Mips[0].BulkData.LockReadOnly());
		auto trData		= static_cast<const FColor*>((*Cast<UTexture2DDynamic>(trTex)		->GetRunningPlatformData())->Mips[0].BulkData.LockReadOnly());

		auto delta = outVerticesWithoutElevation[1 + resolution] - outVerticesWithoutElevation[0];
		
		if (resolution < 1) resolution = 1;
		int i = 0;
		for (int ix = 0; ix < resolution + 1; ix++)
		{
			for (int iy = 0; iy < resolution + 1; iy++)
			{
				if (ix < resolution && iy < resolution)
				{
					outVerticesWithoutElevation[i].Z = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), tileData[h * (w * ix / resolution) *  + (h * iy / resolution)]);
				}
				else if (ix < resolution)
				{
					outVerticesWithoutElevation[i].Z = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), topData[h * (w * ix / resolution) + 0]);
				}
				else if (iy < resolution)
				{
					outVerticesWithoutElevation[i].Z = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), rightData[(h * iy / resolution)]);
				}
				else
				{
					outVerticesWithoutElevation[i].Z = Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), trData[0]);
				}
			}
		}

		for (int ix = 0; ix < resolution + 1; ix++)
		{
			for (int iy = 0; iy < resolution + 1; iy++)
			{
				if (ix < resolution - 1 && iy < resolution - 1)
				{
					outNormals[i] = FVector::CrossProduct(outVerticesWithoutElevation[iy * resolution + ix + 1] - outVerticesWithoutElevation[i], outVerticesWithoutElevation[(iy + 1) * resolution + ix] - outVerticesWithoutElevation[i]).GetSafeNormal();
				}
				else if (ix < resolution - 1)
				{
					float hr =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), rightData[h / resolution]);
					outNormals[i] = FVector::CrossProduct(outVerticesWithoutElevation[iy * resolution + ix + 1] - outVerticesWithoutElevation[i], FVector(0, delta.Y, hr - outVerticesWithoutElevation[i].Z)).GetSafeNormal();
				}
				else if (iy < resolution - 1)
				{
					float hb =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), topData[h / resolution]);
					outNormals[i] = FVector::CrossProduct(outVerticesWithoutElevation[(iy + 1) * resolution + ix] - outVerticesWithoutElevation[i], FVector(delta.X, 0, hb - outVerticesWithoutElevation[i].Z)).GetSafeNormal();
				}
				else
				{
					float hr =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), trData[h / resolution]);
					float hb =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), trData[h * (w / resolution)]);
					outNormals[i] = FVector::CrossProduct(FVector(delta.X, 0, hr - outVerticesWithoutElevation[i].Z), FVector(0, delta.Y, hb - outVerticesWithoutElevation[i].Z)).GetSafeNormal();
				}

			}
		}

		(*((UTexture2DDynamic*)tex)		->GetRunningPlatformData())->Mips[0].BulkData.Unlock();
		(*((UTexture2DDynamic*)rightTex)	->GetRunningPlatformData())->Mips[0].BulkData.Unlock();
		(*((UTexture2DDynamic*)topTex)	->GetRunningPlatformData())->Mips[0].BulkData.Unlock();
		(*((UTexture2DDynamic*)trTex)		->GetRunningPlatformData())->Mips[0].BulkData.Unlock();
	}

private:
	
};