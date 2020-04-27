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

	UFUNCTION(BlueprintCallable, Category = "Default")
	void SetHeightCalculator(TScriptInterface<IHeightCalculator> inCalculator)
	{
		HeightCalculator = inCalculator;
	}

	/** Path to where store cached tiles for further load*/
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	FString CachePath = "DefaultCache";

	/** Name of cache file for tile. Used with format(CacheNameFormat, x, y, z) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
	FString CacheNameFormat = "z{2}x{0}y{1}.tile";	
	
	void GenerateVertices(UTileData* tile, FString channel, int resolution, TArray<FVector>& inoutVertices, TArray<FVector>& outNormals)
	{
		auto right = tile->GetRightNeighbor();
		auto bottom = tile->GetBottomNeighbor();
		auto bottomRight = tile->GetBottomRightNeighbor();
				
		int h = (int)tile->Textures[channel]->GetSurfaceHeight();
		int w = (int)tile->Textures[channel]->GetSurfaceWidth();

		
		auto& tileData		= tile		->HeightMap;
		auto& rightData		= right		->HeightMap;
		auto& bottomData		= bottom		->HeightMap;
		auto& brData			= bottomRight	->HeightMap;

		auto delta = inoutVertices[2 + resolution] - inoutVertices[0];

		auto object = HeightCalculator.GetObject();
		auto interface = Cast<IHeightCalculator>(object);
		
		if (resolution < 1) resolution = 1;
		for (int ix = 0; ix < resolution + 1; ix++)
		{
			for (int iy = 0; iy < resolution + 1; iy++)
			{
				int xNorm = (ix / resolution);
				int yNorm = (iy / resolution);
				if (ix < resolution && iy < resolution)
				{
					inoutVertices[ix * (resolution + 1) + iy].Z = interface->Execute_CalcHeight(object, tileData[((w-1) * ix / resolution) + w*((h-1) * iy / resolution)]);
				}
				else if (ix < resolution)
				{
					inoutVertices[ix * (resolution + 1) + iy].Z = interface->Execute_CalcHeight(object, bottomData[((w-1) * ix / resolution) + 0]);
				}
				else if (iy < resolution)
				{
					inoutVertices[ix * (resolution + 1) + iy].Z = interface->Execute_CalcHeight(object, rightData[0 + w*((h-1) * iy / resolution)]);
				}
				else
				{
					inoutVertices[ix * (resolution + 1) + iy].Z = interface->Execute_CalcHeight(object, brData[0]);
				}
			}
		}
		for (int ix = 0; ix < resolution + 1; ix++)
		{
			for (int iy = 0; iy < resolution + 1; iy++)
			{				
				if (ix < resolution - 1 && iy < resolution - 1)
				{
					FVector r = inoutVertices[(ix + 1) * (resolution + 1) + iy] - inoutVertices[ix * (resolution + 1) + iy];
					FVector b = inoutVertices[ix * (resolution + 1) + iy + 1] - inoutVertices[ix * (resolution + 1) + iy];
					outNormals[ix * (resolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
				}
				else if (ix < resolution - 1)
				{
					float hr =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), bottomData[((w-1) * ix / resolution) + w*((h-1) * 1 / resolution)]);
					FVector r = inoutVertices[(ix + 1) * (resolution + 1) + iy] - inoutVertices[ix * (resolution + 1) + iy];
					FVector b = FVector(0, delta.Y, hr - inoutVertices[ix * (resolution + 1) + iy].Z);
					outNormals[ix * (resolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
				}
				else if (iy < resolution - 1)
				{
					float hb =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), rightData[((w-1) * 1 / resolution) + w*((h-1) * iy / resolution)]);
					FVector r = FVector(delta.X, 0, hb - inoutVertices[ix * (resolution + 1) + iy].Z);
					FVector b = inoutVertices[ix * (resolution + 1) + iy + 1] - inoutVertices[ix * (resolution + 1) + iy];
					outNormals[ix * (resolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
				}
				else
				{
					float hr =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), bottomData[((w-1) * ix / resolution) + w*((h-1) * 1 / resolution)]);
					float hb =  Cast<IHeightCalculator>(HeightCalculator.GetObject())->Execute_CalcHeight(HeightCalculator.GetObject(), rightData[((w-1) * 1 / resolution) + w*((h-1) * iy / resolution)]);
					FVector r = FVector(delta.X, 0, hb - inoutVertices[ix * (resolution + 1) + iy].Z);
					FVector b = FVector(0, delta.Y, hr - inoutVertices[ix * (resolution + 1) + iy].Z);
					outNormals[ix * (resolution + 1) + iy] = FVector::CrossProduct(r, b).GetSafeNormal();
				}
			}
		}
	}

private:
	
};