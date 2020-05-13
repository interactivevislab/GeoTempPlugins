#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "GeometryData.h"
#include "MasksShaderDeclaration.h"

#include "MasksLoader.generated.h"


class BasePolygonPreparer;

/** \class UMaskLoader
 * \brief Utility object for making raster masks from polygons
 * 
 * This class creating raster from input polygons and writes them to render target. The polygons are prepared by PolygonPreparer or its overrides.
 * The polygons after rendered into raster mask with additive color blending (i. e. them are not occluding each other)
 * The polygons can also specify dates of existence, which controls their appearance (you can pass current year into RenderMaskForTime to filter polygons or draw them with interpolation)
 * @see UBasePolygonPreparer
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class POLYGONMASKSGENERATOR_API UMaskLoader : public UActorComponent
{
    GENERATED_BODY()

public:

    /** should update vertex buffer on next rener pass */
    UPROPERTY()
    bool IsDirty;

    /** Rectangle in masks polygon space that will be drawn to render texture */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation")
    FVector4 Rect;

    /** Vertex buffer */
    TArray<FMaskPolygonVertex> Vertices;
    /** index buffer */
    TArray<uint32> Triangles;

    UMaskLoader();
    ~UMaskLoader();

    /** \fn RenderMaskForTime
     * Renders polygons to render target according to current year
     * @param inYear current year to which we should filter polygons
     * @param inTarget render target to render polygons to
     * @param inFraction fraction of year for more precise control (i. e. 0 is 01.01 and 1 is 31.12)
     */
    UFUNCTION(BlueprintCallable)
    void RenderMaskForTime(int inYear, UTextureRenderTarget2D* inTarget, float inFraction);

#pragma region Render Data

protected:

    /** const buffer */
    FPixelShaderConstantParameters ConstantParameters;

    /** variable buffer */
    FPixelShaderVariableParameters VariableParameters;

    /** feature level */
    ERHIFeatureLevel::Type FeatureLevel;

    /** Main texture ref */
    FTexture2DRHIRef CurrentTexture;

    /** Render target to draw mask */
    UPROPERTY()
    UTextureRenderTarget2D* CurrentRenderTarget = nullptr;

    /** vertex buffer */
    FVertexBufferRHIRef VertBuf;
    /** index buffer */
    FIndexBufferRHIRef IndBuf;

    /** Are we already called shader in this frame */
    bool IsPixelShaderExecuting;
    /** Shoud we regenereate SRV */
    bool MustRegenerateSRV;
    /** Is this object currently unloading */
    bool IsUnloading;

protected:

    /** Request to execute pixel shader on next render pass */
    UFUNCTION(BlueprintCallable, Category = "Render Data")
    void ExecutePixelShader(UTextureRenderTarget2D* inInclTarget);

    /** Execute pixel shader from render tread */
    void ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList, bool inIsExclude);

#pragma endregion 

};
