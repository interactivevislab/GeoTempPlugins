#pragma once

#include "CoreMinimal.h"

#include "BlurShaderDeclaration.h"
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "MasksBlur.generated.h"

/** Utility object for bluring textures */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class POLYGONMASKSGENERATOR_API UMaskBlurer : public UActorComponent
{
	GENERATED_BODY()

public:

	UMaskBlurer();
	~UMaskBlurer();

	/** \fn BlurMask
	 * Blur render target and write it into another render target
	 * @param inTargetToWrite render target to write result
	 * @param inTempTarget texture for inner updates. Are not used and can be freely changed outside this blurer render pass so you can use same tempTarget in multiple blurers and other classes. However the data is not stored.
	 * @param inInputTexture render target with texture to blur
	 * @param inDistance blur distance as fraction of texture size
	 * @param inSteps number of steps blur function perform in each axis
	 */
	UFUNCTION(BlueprintCallable)
	void BlurMask(UTextureRenderTarget2D* inTargetToWrite, UTextureRenderTarget2D* inTempTarget, 
		UTextureRenderTarget2D* inInputTexture, float inDistance, int inSteps);
	
#pragma region Render Data

private:

	/** const buffer */
	FPixelShaderConstantParametersBlur	constParams;
	/** variable buffer */
	FPixelShaderVariableParametersBlur	varParams;

	/** feature level */
	ERHIFeatureLevel::Type	featureLevel;

	/** vertex buffer */
	FVertexBufferRHIRef		vertBuf;
	/** output texture ref */
	FTexture2DRHIRef		currentTex;

	/** input texture ref */
	FTexture2DRHIRef		texParam;

	/** inner pointer to outputRenderTarget */
	UPROPERTY()
	UTextureRenderTarget2D* outputRenderTarget = nullptr;

	/** inner pointer to tempRenderTarget */
	UPROPERTY()
	UTextureRenderTarget2D* innerRenderTarget = nullptr;

	/** inner pointer to innerRenderTarget */
	UPROPERTY()
	UTextureRenderTarget2D* inputTarget = nullptr;

	/** Reference to texture Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef texParamSrv;

	/** Are we already called shader in this frame */
	bool isPixelShaderExecuting;
	
	/** Shoud we regenereate SRV */
	bool mustRegenerateSRV;

	/** Is this object currently unloading */
	bool isUnloading;
protected:
	/** Request to execute pixel shader on next render pass */
	UFUNCTION(BlueprintCallable, Category = "Render Data")
	void ExecutePixelShader(UTextureRenderTarget2D* inTarget, UTextureRenderTarget2D* inTexture);

	
	/** Execute pixel shader from render tread */
	void ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList);

#pragma endregion 
};
