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


//class FPositionVertexData :
//	public TStaticMeshVertexData<FMyTextureVertex>
//{
//public:
//	FPositionVertexData(bool InNeedsCPUAccess = false)
//		: TStaticMeshVertexData<FMyTextureVertex>(InNeedsCPUAccess)
//	{
//	}
//};


TResourceArray<FMyTextureVertex>* dataMask;

UMaskLoader::UMaskLoader()
{
	FeatureLevel = ERHIFeatureLevel::SM5;
	FColor StartColor = FColor::Red;

	ConstantParameters = FPixelShaderConstantParameters();	

	VariableParameters = FPixelShaderVariableParameters();

	bMustRegenerateSRV = false;
	bIsPixelShaderExecuting = false;
	bIsUnloading = false;

	CurrentTexture = NULL;
	CurrentRenderTarget = NULL;
	TextureParameterSRV = NULL;
}

UMaskLoader::~UMaskLoader()
{
	bIsUnloading = true;
}

void UMaskLoader::ExecutePixelShader(UTextureRenderTarget2D* inclTarget)
{
	check(IsInGameThread());

	if (bIsUnloading || bIsPixelShaderExecuting) //Skip this execution round if we are already executing
		return;

	if (!inclTarget)
		return;

	bIsPixelShaderExecuting = true;



	//Now set our runtime parameters!
	//VariableParameters.WorldViewProj = FMatrix::Identity;	

	CurrentRenderTarget = inclTarget;
	//CurrentRenderTarget->ClearColor = FLinearColor(0, 1, 0, 0);

	//This macro sends the function we declare inside to be run on the render thread. What we do is essentially just send this class and tell the render thread to run the internal render function as soon as it can.
	//I am still not 100% Certain on the thread safety of this, if you are getting crashes, depending on how advanced code you have in the start of the ExecutePixelShader function, you might have to use a lock :)
	ENQUEUE_RENDER_COMMAND(FPixelShaderRunner)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			ExecutePixelShaderInternal(RHICmdList, true);
			bIsPixelShaderExecuting = false;
		}
	);
}

void UMaskLoader::ExecutePixelShaderInternal(FRHICommandListImmediate& RHICmdList, bool isExclude)
{
	check(IsInRenderingThread());

	if (!CurrentRenderTarget)
		return;
	if (!CurrentRenderTarget->GetRenderTargetResource())
		return;

	if (bIsUnloading) //If we are about to unload, so just clean up the SRV :)
	{
		if (NULL != TextureParameterSRV)
		{
			TextureParameterSRV.SafeRelease();
			TextureParameterSRV = NULL;
		}

		return;
	}

	if (Vertices.Num() == 0) return;
	
	if (!vertBuf.IsValid() || Dirty) {
		Dirty = false;

		dataMask = new TResourceArray<FMyTextureVertex>();
		for (int i = 0; i < Vertices.Num(); i++)
		{
			dataMask->Add(Vertices[i]);
		}
		FRHIResourceCreateInfo info(dataMask);
		//info.bWithoutNativeResource = false;
		vertBuf = RHICreateVertexBuffer(sizeof(FMyTextureVertex) * Vertices.Num(), BUF_Static | BUF_Transient, info);
		

		auto triangles = TResourceArray<uint32>();
		for (int i = 0; i < Triangles.Num(); i++)
		{
			triangles.Add(Triangles[i]);
		}				
		FRHIResourceCreateInfo indInfo(&triangles);
		indBuf = RHICreateIndexBuffer(sizeof(uint32), sizeof(uint32) * Triangles.Num(), BUF_Volatile, indInfo);
	}

	//FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();


	//If our input texture reference has changed, we need to recreate our SRV
	if (bMustRegenerateSRV)
	{
		bMustRegenerateSRV = false;

		if (NULL != TextureParameterSRV)
		{
			TextureParameterSRV.SafeRelease();
			TextureParameterSRV = NULL;
		}

		TextureParameterSRV = RHICreateShaderResourceView(TextureParameter, 0);
	}

	// This is where the magic happens
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(FeatureLevel));
	TShaderMapRef<FPixelShaderExample> PixelShader(GetGlobalShaderMap(FeatureLevel));
	CurrentRenderTarget->UpdateResourceImmediate();	
	CurrentTexture = CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
	
	FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY()), PF_DepthStencil, FClearValueBinding::DepthFar, TexCreate_None, TexCreate_DepthStencilTargetable, false));
	Desc.AutoWritable = false;
	TRefCountPtr<IPooledRenderTarget> DepthRenderTarget;
	GRenderTargetPool.FindFreeElement(RHICmdList, Desc, DepthRenderTarget, TEXT("DepthDummy"), true, ERenderTargetTransience::NonTransient);

	
	//FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::DontLoad_Store, FTextureRHIRef());
	FRHIRenderPassInfo RPInfo(CurrentTexture, ERenderTargetActions::DontLoad_Store, DepthRenderTarget->GetRenderTargetItem().TargetableTexture, EDepthStencilTargetActions::DontLoad_StoreDepthStencil);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("TestTestTest"));
	{
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_InverseDestAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_One>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Max, BF_One, BF_One, BO_Max, BF_One, BF_One>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<true, CF_LessEqual>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclarationMask.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);


		PixelShader->SetOutputTexture(RHICmdList, TextureParameterSRV);		
		VertexShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);
		PixelShader->SetUniformBuffers(RHICmdList, ConstantParameters, VariableParameters);		
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);		
		
		RHICmdList.SetViewport(
			0, 0, -500,
			CurrentTexture->GetSizeX(), CurrentTexture->GetSizeY(), 500.f);
		
		RHICmdList.SetStreamSource(0, vertBuf, 0);		
		
		RHICmdList.DrawIndexedPrimitive(indBuf, 0, 0, Vertices.Num(), 0, Triangles.Num() / 3, 0);
	}
	RHICmdList.EndRenderPass();


	//DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));

	PixelShader->UnbindBuffers(RHICmdList);

	// Resolve render target.
	//RHICmdList.CopyToResolveTarget(
	//	CurrentRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture(),
	//	CurrentRenderTarget->GetRenderTargetResource()->TextureRHI,
	//	FResolveParams());


	//if (bSave) //Save to disk if we have a save request!
	//{
	//	bSave = false;
	//
	//	SaveScreenshot(RHICmdList);
	//}

	bIsPixelShaderExecuting = false;
}


void UMaskLoader::UpdateRect() {
	VariableParameters.Rect = Rect;
}

void UMaskLoader::RenderMaskForTime(int year, UTextureRenderTarget2D* inclTarget, float p)
{
	VariableParameters.Year = year + p;
	VariableParameters.Rect = Rect;
	ExecutePixelShader(inclTarget);
}
