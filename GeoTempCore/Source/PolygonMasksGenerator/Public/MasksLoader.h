#pragma once
#include "PosgisData.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "MasksShaderDeclaration.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MasksLoader.generated.h"

class BasePolygonPreparer;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class POLYGONMASKSGENERATOR_API UMaskLoader : public UActorComponent
{
	GENERATED_BODY()
public:
	UMaskLoader();
	~UMaskLoader();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
		FString PolygonQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
		UTextureRenderTarget2D* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation")
		TArray<int> Years;

	UPROPERTY(/*VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation"*/)
		bool Dirty;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation")
		FVector4 Rect;
	
	TArray<FMyTextureVertex> Vertices;
	TArray<uint32> Triangles;

	UPROPERTY(/*Category = "Inner  Mask Data"*/) TArray<FPosgisContourData> Polygons;
#pragma region Render Data
protected:
	FPixelShaderConstantParameters ConstantParameters;
	FPixelShaderVariableParameters VariableParameters;
	ERHIFeatureLevel::Type FeatureLevel;

	/** Main texture */
	FTexture2DRHIRef CurrentTexture;
	FTexture2DRHIRef TextureParameter;
	UPROPERTY(/*Category = "Render Data"*/) UTextureRenderTarget2D* CurrentRenderTarget = nullptr;
	

	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef TextureParameterSRV;

	FVertexBufferRHIRef vertBuf;
	FIndexBufferRHIRef indBuf;
	
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
	UFUNCTION(BlueprintCallable, Category = "Render Data")
		void ExecutePixelShader(UTextureRenderTarget2D* inclTarget);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList, bool isExclude);

#pragma endregion 

	void UpdateRect();

	/*UFUNCTION()
		void SaveMasksState();*/

	UFUNCTION(BlueprintCallable)
		void RenderMaskForTime(int year, UTextureRenderTarget2D* inclTarget, float p);
};