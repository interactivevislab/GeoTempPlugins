// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorTickable.h"
#include "TextureResource.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "RuntimeMeshComponent.h"
#include "TileGeometryGenerator.h"
#include "TilesBasics.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"

#include "TileVisualizer.generated.h"

class UTileTextureContainer;

class UTileData;

/** Actor component for tiles handling and visualization*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEOTEMPTILES_API UTilesController : public URuntimeMeshComponent, public IEditorTickable
{
    GENERATED_BODY()

public:    
    // Sets default values for this component's properties
    UTilesController(const FObjectInitializer& ObjectInitializer);

protected:

public:

    /** @name Implementation of UActorComponent default functions */
    ///@{    
    bool tickEnabled = false;
    
    void BeginPlay() override;

    void PostLoad();

    void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    ///@}

    /** Do we need to initialize tiles (for example, after loading level) */
    //UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
        static bool NeedInitOnTick;// = true;
    
    /** @name Implementation of IEditorTickable */
    ///@{
    void EditorTick_Implementation(float inDeltaTime) override;;
    ///@}   


    /** Calculate tile screen size in pixels */
    float GetPixelSize(FTileCoordinates inTileCoords);

    /** Geodetic longitude of actor origin */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    float CenterLon;

    /** Geodetic latitude of actor origin */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    float CenterLat;

    /** Create tiles around central point */
    UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
    void CreateMesh();

    /** Clear all tile meshes */
    UFUNCTION(BlueprintCallable, Category = "Tiles", meta = (CallInEditor = "true"))
    void ClearMesh();

    /** Create tiles around specific point in scene space */
    UFUNCTION(BlueprintCallable, Category = "Tiles")
    void CreateMeshAroundPoint(int inZoom, int inX, int inY);

    /** Pointer to tiles material. Material should contain texture parameter called `Tile` */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    UMaterialInterface* TileMaterial;    

    /** Pointer to Tile Texture Container. Inits internally.
     * @see UTileTextureContainer
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    UTileTextureContainer* TileLoader;
    
    /** Array of all Tiles currently loaded*/
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    TMap<FTileCoordinates, UTileData*> Tiles;

    /** Default zoom level for tiles generation */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    int BaseLevel = 10;

    /** Number of base level  tiles generated along single axis */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    int BaseLevelSize = 8;

    /** Max zoom level tiles will split to */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    int MaxLevel = 18;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    int TileMeshResolution = 4;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    UTileGeometryGenerator* GeometryGenerator;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tiles")
    FString ElevationChannel = "Elevation";

    /** Flag to check if files were loaded */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    bool AreTilesLoaded;
    
private: 

    /** free indices in tile array */
    UPROPERTY()
    TArray<int> freeIndices;

    /** Map of tile indices in array */
    UPROPERTY(NoClear)
    TMap<FTileCoordinates, int> TileIndecies;

    /** indices of reserved, but not loaded, tiles */
    UPROPERTY(NoClear)
    TMap<FTileCoordinates, int> ReservedIndecies;

    /** Tiles which are not currently present, but preserved in memory a them are parents of other tiles */
    UPROPERTY()
    TSet<FTileCoordinates> SplitTiles;

    /** Get tile coordinates based on offset vector in scene space
     * @param inOffsetVector offset vector in scene space
     * @param inZ zoom level
     * @param outX x coordinate of the tile
     * @param outY y coordinate of the tile
     */
    UFUNCTION(BlueprintCallable, Category = "Math")
    void GetMercatorXYFromOffset(FVector inOffsetVector, int inZ, int& outX, int& outY);

    /** Get tile coordinates offset from default tile coordinates based on offset vector in scene space
     * @param inOffsetVector offset vector in scene space
     * @param inZ zoom level
     * @param outX x offset of the tile
     * @param outY y offset of the tile
     */
    UFUNCTION(BlueprintCallable, Category = "Math")
    void GetMercatorXYOffsetFromOffset(FVector inOffsetVector, int inZ, int& outX, int& outY);

    /** Get scene coordinates of tile based on it tile coordinates
     * @param inZ zoom level
     * @param inX x coordinate of the tile
     * @param inY y coordinate of the tile
     * @return coordinate of tile center in scene coordinates
     */
    UFUNCTION(BlueprintCallable, Category = "Math")
    FVector GetXYOffsetFromMercatorOffset(int inZ, int inX, int inY);

    /** Calculate mercator x position based on lontitude */
    static float GetMercatorXFromDegrees(double inLon)
    {
        return ((inLon / 180 * PI) + PI) / 2 / PI;
    }    

    /** Calculate mercator x position based on latitude */
    static float GetMercatorYFromDegrees(double inLat)
    {
        return (PI - FMath::Loge(FMath::Tan(PI / 4 + inLat * PI / 180 / 2))) / 2 / PI;
    }

    /** Const earth radius */
    static double EarthRadius;// = 6378.137;
    /** Const one degree on equator length */
    static double EarthOneDegreeLengthOnEquator;// = 111152.8928;
    
    /** queue creation of a mesh section for a tile */
    int BeginCreateTileMesh(int inX, int inY, int inZ);
    
    /** queue creation of a mesh section for a tile */
    int BeginCreateTileMesh(FTileCoordinates inTileCoords, bool inInitNeighbours = true);

    /** create mesh section for a tile */
    UFUNCTION()
    void CreateTileMesh(UTileData* inTile);
    
    /** clear mesh section of a tile */    
    void ClearTileMesh(FTileCoordinates inTileCoords);

    /** check if this tile is currently split */
    bool IsTileSplit(int inX, int inY, int inZ);

    /** split tile */
    void SplitTile(FTileCoordinates inTileCoords);

    /** split tile */
    void SplitTile(int inX, int inY, int inZ);
};
