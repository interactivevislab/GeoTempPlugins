#include "MasksBlur.h"
#include "RHIStaticStates.h"
#include "Engine/Texture2D.h"
#include "PipelineStateCache.h"
#include "StaticMeshVertexData.h"
#include "Engine/StaticMesh.h"
#include "RenderTargetPool.h"

//It seems to be the convention to expose all vertex declarations as globals, and then reference them as externs in the headers where they are needed.
//It kind of makes sense since they do not contain any parameters that change and are purely used as their names suggest, as declarations :)
TGlobalResource<FVertexDeclarationBlur> GTextureVertexDeclarationBlur;


TResourceArray<FBlurTextureVertex>* dataBlur;

UMaskBlurer::UMaskBlurer()
{
	FeatureLevel = ERHIFeatureLevel::SM5;
	FColor StartColor = FColor::Red;

	ConstantParameters = FPixelShaderConstantParametersBlur();	
	
	VariableParameters = FPixelShaderVariableParametersBlur();

	bMustRegenerateSRV = false;
	bIsPixelShaderExecuting = false;
	bIsUnloading = false;

	CurrentTexture = NULL;
	CurrentRenderTarget = NULL;	
	TextureParameterSRV = NULL;
}

UMaskBlurer::~UMaskBlurer()
{
	bIsUnloading = true;
}

void UMaskBlurer::ExecutePixelShader(UTextureRenderTarget2D* Target, UTextureRenderTarget2D* inputTexture)
{
	check(IsInGameThread());

	if (bIsUnloading || bIsPixelShaderExecuting) //Skip this execution round if we are already executing
		return;

	if (!Target)
		return;

	bIsPixelShaderExecuting = true;
			
	CurrentRenderTarget = Target;
	InputTarget = inputTexture;	

	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			ExecutePixelShaderInternal(RHICmdList);
			bIsPixelShaderExecuting = false;
		}
	);
}

void UMaskBlurer::ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList)
{
	check(IsInRenderingThread());


	if (!CurrentRenderTarget)
		return;
	if (!CurrentRenderTarget->GetRenderTargetResource())
		return;
	TextureParameter = InputTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	if (bIsUnloading) //If we are about to unload, so just clean up the SRV :)
	{
		if (NULL != TextureParameterSRV)
		{
			TextureParameterSRV.SafeRelease();
			TextureParameterSRV = NULL;
		}

		return;
	}


	if (!vertBuf.IsValid()) {
		// Draw a fullscreen quad that we can run our pixel shader on
		//TArray<FBlurTextureVertex> Vertices;
		dataBlur = new TResourceArray<FBlurTextureVertex>();
		FBlurTextureVertex vert;
		vert.Position = FVector(-1.0f, 1.0f, 0);
		vert.Color = FColor::Red;
		dataBlur->Add(vert);

		FBlurTextureVertex vert1;
		vert1.Position = FVector4(1.0f, 1.0f, 0, 1.0f);
		vert1.Color = FColor::Green;
		dataBlur->Add(vert1);

		FBlurTextureVertex vert2;
		vert2.Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
		vert2.Color = FColor::Blue;
		dataBlur->Add(vert2);

		FBlurTextureVertex vert3;
		vert3.Position = FVector4(1.0f, -1.0f, 0, 1.0f);
		vert3.Color = FColor::Yellow;
		dataBlur->Add(vert3);

		
		FRHIResourceCreateInfo info(dataBlur);
		vertBuf = RHICreateVertexBuffer(sizeof(FBlurTextureVertex) * dataBlur->Num(), BUF_Static | BUF_Transient, info);
	}

	// This is where the magic happens
	TShaderMapRef<FVertexShaderBlur> VertexShader(GetGlobalShaderMap(FeatureLevel));
	TShaderMapRef<FPixelShaderBlur> PixelShader(GetGlobalShaderMap(FeatureLevel));

	
#pragma region Vertical
	CurrentTexture = InnerRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	if (NULL != TextureParameterSRV)
	{
		TextureParameterSRV.SafeRelease();
		TextureParameterSRV = NULL;
	}
	TextureParameterSRV = RHICreateShaderResourceView(TextureParameter, 0);
	PixelShader->SetInputTexture(RHICmdList, TextureParameterSRV);
	
	VariableParameters.Sigma = 1;
	VariableParameters.Direction = FVector2D(0, 1);
	FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	RHICmdList.BeginRenderPass(RPInfo, TEXT("TestTestTest"));
	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclarationBlur.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		RHICmdList.SetViewport(
			0, 0, 0.f,
			CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY(), 1.f);

		RHICmdList.SetStreamSource(0, vertBuf, 0);
		
		VertexShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
		PixelShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
		RHICmdList.DrawPrimitive(0, 2, 0);		
		
	}
	RHICmdList.EndRenderPass();
#pragma endregion
	
#pragma region Horizontal
	//InnerRenderTarget->UpdateResourceImmediate(false);
	
	CurrentTexture = CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	
	if (NULL != TextureParameterSRV)
	{
		TextureParameterSRV.SafeRelease();
		TextureParameterSRV = NULL;
	}
	auto tRenderTargetTexture = InnerRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	//tRenderTargetTexture->TextureRHI
	TextureParameterSRV = RHICreateShaderResourceView(tRenderTargetTexture, 0);
	
	//TextureParameterSRV = RHICreateShaderResourceView(TextureParameter, 0);
	

	VariableParameters.Sigma = 1;
	VariableParameters.Direction = FVector2D(1, 0);
	
	FRHIRenderPassInfo RPInfo2(CurrentTexture, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	RHICmdList.BeginRenderPass(RPInfo2, TEXT("TestTestTest"));
	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclarationBlur.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		RHICmdList.SetViewport(
			0, 0, 0.f,
			CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY(), 1.f);


		RHICmdList.SetStreamSource(0, vertBuf, 0);

		PixelShader->SetInputTexture(RHICmdList, TextureParameterSRV);
		VertexShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
		PixelShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
		RHICmdList.DrawPrimitive(0, 2, 0);

	}
	RHICmdList.EndRenderPass();
	
#pragma endregion
	


	//DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));

	//VertexShader->UnbindBuffers(RHICmdList);
	PixelShader->UnbindBuffers(RHICmdList);

	// Resolve render target.
	//RHICmdList.CopyToResolveTarget(
	//	CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(),
	//	CurrentRenderTarget->GetRenderTargetResource()->TextureRHI,
	//	FResolveParams());

	bIsPixelShaderExecuting = false;
}


void UMaskBlurer::BlurMask(UTextureRenderTarget2D* Target, UTextureRenderTarget2D* tempTarget, UTextureRenderTarget2D* inputTexture, float distance, int steps)
{
	VariableParameters.Steps = steps;
	VariableParameters.Distance = distance;
	InnerRenderTarget = tempTarget;
	ExecutePixelShader(Target, inputTexture);
}
