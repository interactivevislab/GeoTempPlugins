#pragma once

#include "CoreMinimal.h"

#include "BlurShaderDeclaration.h"
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "MasksBlur.generated.h"


UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class POLYGONMASKSGENERATOR_API UMaskBlurer : public UActorComponent
{
	GENERATED_BODY()

public:

	UMaskBlurer();
	~UMaskBlurer();

	UFUNCTION(BlueprintCallable)
	void BlurMask(UTextureRenderTarget2D* inTexTarget, UTextureRenderTarget2D* inTempTarget, 
		UTextureRenderTarget2D* inInputTexture, float inDistance, int inSteps);
	
#pragma region Render Data

private:

	FPsConstParamsBlur		constParams;
	FPsVarParamsBlur		varParams;
	ERHIFeatureLevel::Type	featureLevel;
	FVertexBufferRHIRef		vertBuf;
	FTexture2DRHIRef		currentTex; //Main texture
	FTexture2DRHIRef		texParam;

	UPROPERTY()
	UTextureRenderTarget2D* currentRenderTarget = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* innerRenderTarget = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* inputTarget = nullptr;

	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef texParamSrv;
	
	FIndexBufferRHIRef indBuf1;
	FIndexBufferRHIRef indBuf2;
	
	bool isPixelShaderExecuting;
	bool mustRegenerateSRV;
	bool isUnloading;
	
/********************************************************************************************************/
/* Let the user change rendertarget during runtime if they want to :D                                   */
/* @param RenderTarget - This is the output rendertarget!                                               */
/* @param InputTexture - This is a rendertarget that's used as a texture parameter to the shader :)     */
/* @param EndColor - This will be set to the dynamic parameter buffer each frame                        */
/* @param TextureParameterBlendFactor - The scalar weight that decides how much of the texture to blend */
/********************************************************************************************************/
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	UTextureRenderTarget2D* BlurTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	float BlurDistance;

	UFUNCTION(BlueprintCallable, Category = "Render Data")
	void ExecutePixelShader(UTextureRenderTarget2D* inTarget, UTextureRenderTarget2D* inTexture);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList);

#pragma endregion 
};