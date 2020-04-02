#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PosgisData.h"
#include "MasksShaderDeclaration.h"

#include "MasksLoader.generated.h"


class BasePolygonPreparer;


UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class POLYGONMASKSGENERATOR_API UMaskLoader : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	FString PolygonQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	UTextureRenderTarget2D* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	TArray<int> Years;

	UPROPERTY()
	bool Dirty;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Masks Generation")
	FVector4 Rect;
	
	TArray<FMyTextureVertex> Vertices;
	TArray<uint32> Triangles;

	UPROPERTY()
	TArray<FPosgisContourData> Polygons;

	UMaskLoader();
	~UMaskLoader();

	void UpdateRect();

	UFUNCTION(BlueprintCallable)
	void RenderMaskForTime(int inYear, UTextureRenderTarget2D* inInclTarget, float inP);

#pragma region Render Data

protected:

	FPsConstParams ConstantParameters;
	FPsVarParams VariableParameters;
	ERHIFeatureLevel::Type FeatureLevel;

	/** Main texture */
	FTexture2DRHIRef CurrentTexture;
	FTexture2DRHIRef TextureParameter;

	UPROPERTY()
	UTextureRenderTarget2D* CurrentRenderTarget = nullptr;

	/** Since we are only reading from the resource, we do not need a UAV; an SRV is sufficient */
	FShaderResourceViewRHIRef TextureParameterSRV;

	FVertexBufferRHIRef VertBuf;
	FIndexBufferRHIRef IndBuf;
	
	bool IsPixelShaderExecuting;
	bool MustRegenerateSRV;
	bool IsUnloading;
	
/********************************************************************************************************/
/* Let the user change rendertarget during runtime if they want to :D                                   */
/* @param RenderTarget - This is the output rendertarget!                                               */
/* @param InputTexture - This is a rendertarget that's used as a texture parameter to the shader :)     */
/* @param EndColor - This will be set to the dynamic parameter buffer each frame                        */
/* @param TextureParameterBlendFactor - The scalar weight that decides how much of the texture to blend */
/********************************************************************************************************/
public:

	UFUNCTION(BlueprintCallable, Category = "Render Data")
	void ExecutePixelShader(UTextureRenderTarget2D* inInclTarget);

	/************************************************************************/
	/* Only execute this from the render thread!!!                          */
	/************************************************************************/
	void ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList, bool inIsExclude);

#pragma endregion 

};