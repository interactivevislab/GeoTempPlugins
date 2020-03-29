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
	
#pragma region Render Data
private:
	FPixelShaderConstantParametersBlur ConstantParameters;
	FPixelShaderVariableParametersBlur VariableParameters;
	ERHIFeatureLevel::Type FeatureLevel;
	FVertexBufferRHIRef vertBuf;
	/** Main texture */
	FTexture2DRHIRef CurrentTexture;
	FTexture2DRHIRef TextureParameter;
	UPROPERTY(/*Category = "Render Data"*/) UTextureRenderTarget2D* CurrentRenderTarget = nullptr;	
	UPROPERTY(/*Category = "Render Data"*/) UTextureRenderTarget2D* InnerRenderTarget = nullptr;	
	UPROPERTY(/*Category = "Render Data"*/) UTextureRenderTarget2D* InputTarget = nullptr;	
	

	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef TextureParameterSRV;
	
	FIndexBufferRHIRef indBuf1;
	FIndexBufferRHIRef indBuf2;
	
	bool bIsPixelShaderExecuting;
	bool bMustRegenerateSRV;
	bool bIsUnloading;
	
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
		void ExecutePixelShader(UTextureRenderTarget2D* target, UTextureRenderTarget2D* texture);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList);

#pragma endregion 	
	/*UFUNCTION()
		void SaveMasksState();*/

	UFUNCTION(BlueprintCallable)
		void BlurMask(UTextureRenderTarget2D* texTarget, UTextureRenderTarget2D* tempTarget, UTextureRenderTarget2D* InputTexture, float distance, int steps);
};