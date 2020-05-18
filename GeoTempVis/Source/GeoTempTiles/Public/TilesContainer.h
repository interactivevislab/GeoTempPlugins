#pragma once

#include "CoreMinimal.h"
#include "EditorTickable.h"
#include "ITileProvider.h"
#include "TilesBasics.h"
#include "Kismet/KismetSystemLibrary.h"


#include "TilesContainer.generated.h"

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class UTileTextureContainer : public UObject, public IEditorTickable
{
    GENERATED_BODY()
    
protected: 


    /** Map of currently loaded and cached tiles */
    UPROPERTY(Transient)
    TMap< FTileCoordinates, UTileData*> CachedTiles;

    /** Mapping of texture getters on channels*/
    UPROPERTY()
    TMap<FString, TScriptInterface<ITileProvider>> TextureGetters;
    
public:
    friend class UTextureDownloader;
    friend class UTileData;

    /** Channel to use as elevation heightmap */
    FString ElevationChannel;

    /** Assign texture getter for a channel */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void AddTextureGetter(FString inChannel, TScriptInterface<ITileProvider> inGetter);

    /** Mark tile as inActive for ram tile cache manager or mark the last usage time. Currently WIP
     * @param inTileCoords tile coordinates structure
     * @param inActive is this tile active now
     */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void SetTileActive(FTileCoordinates inTileCoords, bool inActive);

    /** Create uninitialized tile material instance for provided tile and start downloading texture
     * @param inTileCoords tile coordinates structure
     * @param inMat pointer to default tile material
     * @param inOwner actor working as outer in material instance creation
     */
    UFUNCTION(BlueprintCallable, Category = "Default")
    UTileData* GetTileMaterial(FTileCoordinates inTileCoords, UMaterialInterface* inMat, AActor* inOwner);

    /** Check if texture already downloaded and cached
     * @param inTileCoords tile coordinates structure
     */
    UFUNCTION(BlueprintCallable, Category = "Default")
    bool IsTextureLoaded(FTileCoordinates inTileCoords);

    /** Clear all caches */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void Clear();

    /** Clear all corrupted data in caches */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void CleanMess();


    /** @name Implementation of IEditorTickable */
    ///@{
    void EditorTick_Implementation(float inDeltaTime) override
    {
        for (auto textureGetter : TextureGetters)
        {
            auto object = textureGetter.Value.GetObject();
            if (UKismetSystemLibrary::DoesImplementInterface(object, UEditorTickable::StaticClass()))
            {
                const auto& interface = Cast<IEditorTickable>(object);
                interface->Execute_EditorTick(object, inDeltaTime);
            }
        }
    };
    ///@}
    
private:
    /** Add texture to tile cache
     * @param inTileCoords tile coordinates structure
     * @param inTexture texture to cache
     * @param inChannel texture channel
     * @param inData byte array of texture pixels data
     */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void CacheTexture(FTileCoordinates inTileCoords, UTexture* inTexture, FString inChannel, const TArray<uint8>& inData);
};