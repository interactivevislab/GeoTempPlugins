#include "MasksLoader.h"

#include "RHIStaticStates.h"
#include "Engine/Texture2D.h"
#include "PipelineStateCache.h"
#include "StaticMeshVertexData.h"
#include "Engine/StaticMesh.h"
#include "RenderTargetPool.h"


//It seems to be the convention to expose all vertex declarations as globals, and then reference them as externs in the headers where they are needed.
//It kind of makes sense since they do not contain any parameters that change and are purely used as their names suggest, as declarations :)
TGlobalResource<FVertexDeclarationExample> GTextureVertexDeclarationMask;


TResourceArray<FMaskPolygonVertex>* dataMask;


UMaskLoader::UMaskLoader()
{
	FeatureLevel = ERHIFeatureLevel::SM5;

	ConstantParameters = FPixelShaderConstantParameters();
	VariableParameters = FPixelShaderVariableParameters();

	MustRegenerateSRV		= false;
	IsPixelShaderExecuting	= false;
	IsUnloading				= false;

	CurrentTexture		= nullptr;
	CurrentRenderTarget = nullptr;
}


UMaskLoader::~UMaskLoader()
{
	IsUnloading = true;
}


void UMaskLoader::ExecutePixelShader(UTextureRenderTarget2D* inInclTarget)
{
	check(IsInGameThread());

	if (IsUnloading || IsPixelShaderExecuting || !inInclTarget)
	{
		return;
	}

	IsPixelShaderExecuting = true;

	CurrentRenderTarget = inInclTarget;

	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)
	(
		[&](FRHICommandListImmediate& outRhiCmdList)
		{
			ExecutePixelShaderInternal(outRhiCmdList, true);
			IsPixelShaderExecuting = false;
		}
	);
}


void UMaskLoader::ExecutePixelShaderInternal(FRHICommandListImmediate& outRhiCmdList, bool inIsExclude)
{
	check(IsInRenderingThread());

	if (!CurrentRenderTarget || !CurrentRenderTarget->GetRenderTargetResource())
	{
		return;
	}

	//If we are about to unload, so just clean up the SRV
	if (IsUnloading) 
	{
		return;
	}

	if (Vertices.Num() == 0)
	{
		return;
	}
		
	
	if (!VertBuf.IsValid() || IsDirty)
	{
		IsDirty = false;

		dataMask = new TResourceArray<FMaskPolygonVertex>();
		for (int i = 0; i < Vertices.Num(); i++)
		{
			dataMask->Add(Vertices[i]);
		}

		FRHIResourceCreateInfo info(dataMask);
		VertBuf = RHICreateVertexBuffer(sizeof(FMaskPolygonVertex) * Vertices.Num(), BUF_Static | BUF_Transient, info);

		auto triangles = TResourceArray<uint32>();
		for (int i = 0; i < Triangles.Num(); i++)
		{
			triangles.Add(Triangles[i]);
		}

		FRHIResourceCreateInfo indInfo(&triangles);
		IndBuf = RHICreateIndexBuffer(sizeof(uint32), sizeof(uint32) * Triangles.Num(), BUF_Volatile, indInfo);
	}

	// This is where the magic happens
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(FeatureLevel));
	TShaderMapRef<FPixelShaderExample>	PixelShader	(GetGlobalShaderMap(FeatureLevel));
	CurrentRenderTarget->UpdateResourceImmediate();	
	CurrentTexture = CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	
	FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(CurrentTexture->GetSizeX(), 
		CurrentTexture->GetSizeY()), PF_DepthStencil, FClearValueBinding::DepthFar, 
		TexCreate_None, TexCreate_DepthStencilTargetable, false));

	Desc.AutoWritable = false;
	TRefCountPtr<IPooledRenderTarget> DepthRenderTarget;
	GRenderTargetPool.FindFreeElement(outRhiCmdList, Desc, DepthRenderTarget, TEXT("DepthDummy"), 
		true, ERenderTargetTransience::NonTransient);

	FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::DontLoad_Store, 
		DepthRenderTarget->GetRenderTargetItem().TargetableTexture, 
		EDepthStencilTargetActions::DontLoad_StoreDepthStencil);

	outRhiCmdList.BeginRenderPass(RPInfo, TEXT("TestTestTest"));

	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		outRhiCmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		GraphicsPSOInit.BlendState			= TStaticBlendState<CW_RGBA, BO_Max, BF_One, BF_One, BO_Max, BF_One, BF_One>::GetRHI();
		GraphicsPSOInit.RasterizerState		= TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState	= TStaticDepthStencilState<true, CF_LessEqual>::GetRHI();
		GraphicsPSOInit.PrimitiveType		= PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI	= GTextureVertexDeclarationMask.VertexDeclarationRhi;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI		= VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI			= PixelShader.GetPixelShader();

		VertexShader->SetUniformBuffers(outRhiCmdList, ConstantParameters, VariableParameters);
		PixelShader	->SetUniformBuffers(outRhiCmdList, ConstantParameters, VariableParameters);
		SetGraphicsPipelineState(outRhiCmdList, GraphicsPSOInit);
		
		outRhiCmdList.SetViewport(0, 0, -500, CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY(), 500.f);
		outRhiCmdList.SetStreamSource(0, VertBuf, 0);
		outRhiCmdList.DrawIndexedPrimitive(IndBuf, 0, 0, Vertices.Num(), 0, Triangles.Num() / 3, 0);
	}

	outRhiCmdList.EndRenderPass();

	PixelShader->UnbindBuffers(outRhiCmdList);

	IsPixelShaderExecuting = false;
}


void UMaskLoader::RenderMaskForTime(int inYear, UTextureRenderTarget2D* inTarget, float inFraction)
{
	VariableParameters.Year = inYear + inFraction;
	VariableParameters.Rect = Rect;
	ExecutePixelShader(inTarget);
}
