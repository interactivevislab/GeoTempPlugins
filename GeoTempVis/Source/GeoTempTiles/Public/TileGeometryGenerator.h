#pragma once

#include "CoreMinimal.h"
#include "TilesBasics.h"
#include "HeightCalculators.h"
#include "TileGeometryGenerator.generated.h"

/** Utility object for handling of tile geometry and elevation and data caching*/
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOTEMPTILES_API UTileGeometryGenerator : public UObject
{
    GENERATED_BODY()

    /** Height calculator to determine height based on texure value */
    //UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
    UPROPERTY()
    TScriptInterface<IHeightCalculator> HeightCalculator;
public:
    

    /** Set active height calculator
     * @param inCalculator pointer to chosen calculator
     */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void SetHeightCalculator(TScriptInterface<IHeightCalculator> inCalculator);

    ///** Path to where store cached tiles for further load*/
    //UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
    //FString CachePath = "DefaultCache";

    ///** Name of cache file for tile. Used with format(CacheNameFormat, x, y, z) */
    //UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category= "Default")
    //FString CacheNameFormat = "z{2}x{0}y{1}.tile";    

    /** @fn Invoke this function to write heights into existing vertice array and generate normals
     * @param inTile TileData object storing tile data
     * @param inChannel channel for elevation texture
     * @param inResolution resolution of tile mesh
     * @param inoutVertices array of vertices to write heights into
     * @param outNormals generated array of normals for vertices
     */
    UFUNCTION()
    void GenerateVertices(UTileData* inTile, FString inChannel, int inResolution, TArray<FVector>& inoutVertices,
                        TArray<FVector>& outNormals);
	
};

