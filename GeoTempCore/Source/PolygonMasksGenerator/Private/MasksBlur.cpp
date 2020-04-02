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
	featureLevel = ERHIFeatureLevel::SM5;

	constParams = FPsConstParamsBlur();
	varParams	= FPsVarParamsBlur();

	mustRegenerateSRV		= false;
	isPixelShaderExecuting	= false;
	isUnloading				= false;

	currentTex			= nullptr;
	currentRenderTarget = nullptr;	
	texParamSrv			= nullptr;
}


UMaskBlurer::~UMaskBlurer()
{
	isUnloading = true;
}


void UMaskBlurer::ExecutePixelShader(UTextureRenderTarget2D* inTarget, UTextureRenderTarget2D* inTexture)
{
	check(IsInGameThread());

	if (isUnloading || isPixelShaderExecuting || !inTarget)
	{
		return;
	}

	isPixelShaderExecuting = true;
			
	currentRenderTarget = inTarget;
	inputTarget = inTexture;

	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)
	(
		[&](FRHICommandListImmediate& outRhiCmdList)
		{
			ExecutePixelShaderInternal(outRhiCmdList);
			isPixelShaderExecuting = false;
		}
	);
}


void UMaskBlurer::ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList)
{
	check(IsInRenderingThread());

	if (!currentRenderTarget || !currentRenderTarget->GetRenderTargetResource())
	{
		return;
	}

	texParam = inputTarget->GetRenderTargetResource()->GetRenderTargetTexture();

	//If we are about to unload, so just clean up the SRV
	if (isUnloading)
	{
		if (texParamSrv != nullptr)
		{
			texParamSrv.SafeRelease();
			texParamSrv = nullptr;
		}

		return;
	}

	if (!vertBuf.IsValid())
	{
		// Draw a fullscreen quad that we can run our pixel shader on
		dataBlur = new TResourceArray<FBlurTextureVertex>();

		FBlurTextureVertex vert0, vert1, vert2, vert3;

		vert0.Position = FVector4(-1.0f,  1.0f, 0, 1.0f);
		vert1.Position = FVector4( 1.0f,  1.0f, 0, 1.0f);
		vert2.Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
		vert3.Position = FVector4( 1.0f, -1.0f, 0, 1.0f);

		vert0.Color = FColor::Red;
		vert1.Color = FColor::Green;
		vert2.Color = FColor::Blue;
		vert3.Color = FColor::Yellow;

		dataBlur->Add(vert0);
		dataBlur->Add(vert1);
		dataBlur->Add(vert2);
		dataBlur->Add(vert3);
		
		FRHIResourceCreateInfo info(dataBlur);
		vertBuf = RHICreateVertexBuffer(sizeof(FBlurTextureVertex) * dataBlur->Num(), BUF_Static | BUF_Transient, info);
	}

	// This is where the magic happens
	TShaderMapRef<FVertexShaderBlur>	vertexShader(GetGlobalShaderMap(featureLevel));
	TShaderMapRef<FPixelShaderBlur>		pixelShader (GetGlobalShaderMap(featureLevel));

	
#pragma region Vertical

	currentTex = innerRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	if (texParamSrv != nullptr)
	{
		texParamSrv.SafeRelease();
		texParamSrv = nullptr;
	}
	texParamSrv = RHICreateShaderResourceView(texParam, 0);
	pixelShader->SetInputTexture(outRhiCmdList, texParamSrv);
	
	varParams.Sigma = 1;
	varParams.Direction = FVector2D(0, 1);
	FRHIRenderPassInfo rpInfo(currentTex, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	outRhiCmdList.BeginRenderPass(rpInfo, TEXT("TestTestTest"));

	{
		FGraphicsPipelineStateInitializer graphicsPsoInit;
		outRhiCmdList.ApplyCachedRenderTargets(graphicsPsoInit);

		graphicsPsoInit.BlendState			= TStaticBlendState<>::GetRHI();
		graphicsPsoInit.RasterizerState		= TStaticRasterizerState<>::GetRHI();
		graphicsPsoInit.DepthStencilState	= TStaticDepthStencilState<false, CF_Always>::GetRHI();
		graphicsPsoInit.PrimitiveType		= PT_TriangleStrip;
		graphicsPsoInit.BoundShaderState.VertexDeclarationRHI	= GTextureVertexDeclarationBlur.VertexDeclarationRhi;
		graphicsPsoInit.BoundShaderState.VertexShaderRHI		= GETSAFERHISHADER_VERTEX(*vertexShader);
		graphicsPsoInit.BoundShaderState.PixelShaderRHI			= GETSAFERHISHADER_PIXEL(*pixelShader);

		SetGraphicsPipelineState(outRhiCmdList, graphicsPsoInit);

		outRhiCmdList.SetViewport(0, 0, 0.f, currentTex->GetSizeX(), currentTex->GetSizeY(), 1.f);
		outRhiCmdList.SetStreamSource(0, vertBuf, 0);
		
		vertexShader->SetUniformBuffers(outRhiCmdList, constParams, varParams);
		pixelShader	->SetUniformBuffers(outRhiCmdList, constParams, varParams);
		outRhiCmdList.DrawPrimitive(0, 2, 0);
	}

	outRhiCmdList.EndRenderPass();

#pragma endregion
	
#pragma region Horizontal
	
	currentTex = currentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	
	if (texParamSrv != nullptr)
	{
		texParamSrv.SafeRelease();
		texParamSrv = nullptr;
	}
	auto tRenderTargetTexture = innerRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	texParamSrv = RHICreateShaderResourceView(tRenderTargetTexture, 0);

	varParams.Sigma = 1;
	varParams.Direction = FVector2D(1, 0);
	
	FRHIRenderPassInfo rpInfo2(currentTex, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	outRhiCmdList.BeginRenderPass(rpInfo2, TEXT("TestTestTest"));

	{
		FGraphicsPipelineStateInitializer graphicsPsoInit;
		outRhiCmdList.ApplyCachedRenderTargets(graphicsPsoInit);

		graphicsPsoInit.BlendState			= TStaticBlendState<>::GetRHI();
		graphicsPsoInit.RasterizerState		= TStaticRasterizerState<>::GetRHI();
		graphicsPsoInit.DepthStencilState	= TStaticDepthStencilState<false, CF_Always>::GetRHI();
		graphicsPsoInit.PrimitiveType		= PT_TriangleStrip;
		graphicsPsoInit.BoundShaderState.VertexDeclarationRHI	= GTextureVertexDeclarationBlur.VertexDeclarationRhi;
		graphicsPsoInit.BoundShaderState.VertexShaderRHI		= GETSAFERHISHADER_VERTEX(*vertexShader);
		graphicsPsoInit.BoundShaderState.PixelShaderRHI			= GETSAFERHISHADER_PIXEL(*pixelShader);

		SetGraphicsPipelineState(outRhiCmdList, graphicsPsoInit);

		outRhiCmdList.SetViewport(0, 0, 0.f, currentTex->GetSizeX(), currentTex->GetSizeY(), 1.f);
		outRhiCmdList.SetStreamSource(0, vertBuf, 0);

		pixelShader	->SetInputTexture(outRhiCmdList, texParamSrv);
		vertexShader->SetUniformBuffers(outRhiCmdList, constParams, varParams);
		pixelShader	->SetUniformBuffers(outRhiCmdList, constParams, varParams);
		outRhiCmdList.DrawPrimitive(0, 2, 0);

	}
	outRhiCmdList.EndRenderPass();
	
#pragma endregion

	pixelShader->UnbindBuffers(outRhiCmdList);

	isPixelShaderExecuting = false;
}


void UMaskBlurer::BlurMask(UTextureRenderTarget2D* inTexTarget, UTextureRenderTarget2D* inTempTarget,
	UTextureRenderTarget2D* inInputTexture, float inDistance, int inSteps)
{
	varParams.Steps = inSteps;
	varParams.Distance = inDistance;
	innerRenderTarget = inTempTarget;
	ExecutePixelShader(inTexTarget, inInputTexture);
}
