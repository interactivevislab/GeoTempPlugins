// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FoliageInfo.h"
#include "FoliageActor.h"
#include "GeometryData.h"
#include "CustomFoliageInstancer.generated.h"


/**
 * \enum ELayersOption
 *
 * Ways to handle multiple FoliageMeshes during meshes placement.
 */
UENUM(BlueprintType)        
enum class ELayersOption : uint8
{
    MonoLayered        UMETA(DisplayName = "MonoLayered"), /** < Place meshes from multiple FoliageMeshes as one layer. 
                                                        * Results in using one FoliageMesh per cell. 
                                                        */  
    PolyLayered        UMETA(DisplayName = "PolyLayered"), /** < Place meshes from multiple FoliageMeshes as multiple layers. 
                                                        * Results in using all FoliageMeshes per cell. 
                                                        */ 
};

/**
* \class UCustomFoliageInstancer
* \brief Custom mesh instancer.
*
* Custom instancer to place meshes using texture masks.
*/
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class GEOTEMPFOLIAGE_API UCustomFoliageInstancer : public UActorComponent
{
    GENERATED_BODY()

public:
    UCustomFoliageInstancer();

private:
    /** 
    * Buffered InitialTarget pixels. 
    *
    * @see UCustomFoliageInstancer.InitialTarget
    */
    TArray<FColor> colorBuffer;

    /** 
    * Determines the neccessity to update EndTarget. 
    *
    * @see UCustomFoliageInstancer.EndTarget
    */
    bool updateSecondRenderTarget;

    /** Which mask to renter according to current time. */
    int currentMaskIndex;

    /** Array of dates accociated with foliage areas to render to masks. */
    TArray<int> polygonDates;

    /** An actor to contain foliage instancer component. */
    UPROPERTY()
    AFoliageActor* foliageActor = nullptr;

protected:
    virtual void BeginPlay() override;

public:

    /** The X-azis size of instancing area. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    float Width;

    /** The Y-azis size of instancing area. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    float Height;

    /** The size of cell to place mesh into. The less cell size gets the more dence foliage becomes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    FVector2D CellSize;

    /** The angle to rotate instancer. Affects cell rows direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings", meta = (UIMin = "0.0", UIMax = "360.0"))
    float SpawnAngle;

    /** The offset of instancer. Affects cells position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    FVector2D SpawnTranslate;

    /** The seed of the instancer. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    float InstancerSeed;

    /** How to handle multiple FoliageMeshes during meshes placement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    ELayersOption MeshLayersOption;

    /** The mask that shows start state of foliage polygons to inetrpolate from. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    UTextureRenderTarget2D* StartTarget;

    /** The mask that shows end state of foliage polygons to inetrpolate to. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    UTextureRenderTarget2D* EndTarget;

    /** The mask that shows density state of foliage polygons at current time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    UTextureRenderTarget2D* TypesTarget;

    /** The mask that shows presence state of foliage polygons at all times. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InstancerSettings")
    UTextureRenderTarget2D* InitialTarget;

    /** An array of meshes to spawn with their instancing prameters. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
    TArray<FFoliageMeshInfo> FoliageMeshes;

    /** An map to track spawned meshes and their respective Hierarchical instanced static mesh components. */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
    TMap<UStaticMesh*, UHierarchicalInstancedStaticMeshComponent*> FoliageInstancers;

    /** Tags to seek in polygon data to use in mask rendering. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FoliageSettings")
    TMap<FString, FString> DataBaseTags;

    /** 
    * An array of objects to check collision with. 
    *
    * OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConfigurationSettings")
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectQuery;

    /**
    * DEBUG. Whether to draw trace lines for collision check.
    *
    * OBSOLETE PROPERTY. POSSIBLE USE IN FUTURE WITH LANDSCAPE.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool DrawGizmo;

    /** 
    * Current interpolation between StartTarget and EndTarget. 
    *
    * @see UCustomFoliageInstancer.StartTarget
    * @see UCustomFoliageInstancer.EndTarget
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (UIMin = "0.0", UIMax = "1.0"))
    float CurrentInterpolation;

    /**
    * \fn FillFoliage_BP
    * \brief Blueprint function to initiate mesh instancing.
    *
    * Generates instancer components, buffers InitialTarget pixels, creates dynamic materials and sets material parameters for consequent mesh instancing.
    * @param inComponentRect    Area size for instancing.
    * @see UCustomFoliageInstancer.FillFoliageWithMeshes 
    */
    UFUNCTION(BlueprintCallable)
    void FillFoliage_BP(FVector4 inComponentRect);

    /**
    * \fn ClearFoliage_BP
    * \brief Blueprint function to initiate meshes removal.
    *
    * Clears instancers' meshes and destroys components.
    */
    UFUNCTION(BlueprintCallable)
    void ClearFoliage_BP();

    /**
    * \fn FillFoliageWithMeshes
    * \brief Function to instance meshes.
    *
    * Adds meshes to instancers according to polygons rendered in InitialTarget.
    * @param inInfos    An array of FFoliageMeshInfo to use in use in current layer.
    * @param inInstancers    An array of inInstancers to refer to to add new meshes.
    */
    UFUNCTION()
    void FillFoliageWithMeshes(
        TArray<FFoliageMeshInfo>& inInfos, 
        TArray<UHierarchicalInstancedStaticMeshComponent*>& inInstancers
    );
    
    /**
    * \fn InterpolateFoliageWithMaterial
    * \brief Blueprint function to apply current interpolation to mesh instancer material parameter.
    */
    UFUNCTION(BlueprintCallable)
    void InterpolateFoliageWithMaterial();
    
    /**
    * \fn UpdateBuffer
    * \brief Creates pixel buffer from InitialTarget.
    *
    * @see UCustomFoliageInstancer.InitialTarget
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void UpdateBuffer();

    /**
    * \fn UpdateFoliageMasksDates
    * \brief Updates dates to render foliage state for on StartTarget and EndTarget according to current time.
    *
    * @param inCurrentTime            Time to render foliage state for.
    * @param outRenderYearFirst        Date to render foliage state for on StartTarget.
    * @param outRenderYearSecond    Date to render foliage state for on EndTarget.
    * @param outUpdateFirstTarget    Whether need to update StartTarget first.
    * @see UCustomFoliageInstancer.StartTarget
    * @see UCustomFoliageInstancer.EndTarget
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void UpdateFoliageMasksDates(FDateTime inCurrentTime, int& outRenderYearFirst, int& outRenderYearSecond, bool& outUpdateFirstTarget);

    /**
    * \fn GetDatesNearCurrent
    * \brief Checks for closest dates to current time.
    *
    * @param inCurrentTime        Time to compare dates with.
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void GetDatesNearCurrent(FDateTime inCurrentTime);

    /**
    * \fn ParseDates
    * \brief Gets dates from contours and sorts them by ascending.
    *
    * @param inContours        Contours with dates to parse.
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void ParseDates(TArray<FMultipolygonData>& inContours);

    /**
    * \fn ParseTimeTags
    * \brief Gets dates from contours by tag.
    *
    * @param inContour        Contours with dates to parse.
    * @param outDates        A set of unique dates accosiated with polygons changes events.
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void ParseTimeTags(const FMultipolygonData& inContour, TSet<int>& outDates);

    /**
    * \fn SortDatesByAscend
    * \brief Sorts dates from contours by ascending.
    *
    * @param inDates        A set of dated to sort.
    */
    UFUNCTION(BlueprintCallable, Category = "Default")
    void SortDatesByAscend(TSet<int>& inDates);
};
