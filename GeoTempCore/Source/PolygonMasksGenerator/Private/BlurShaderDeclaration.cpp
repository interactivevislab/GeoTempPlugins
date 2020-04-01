#include "BlurShaderDeclaration.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "RHIResources.h"


IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPsConstParamsBlur, "PSConstantB");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPsVarParamsBlur, "PSVariableB");


void FVertexDeclarationBlur::InitRHI()
{
	FVertexDeclarationElementList elements;
	uint32 stride = sizeof(FBlurTextureVertex);
	elements.Add(FVertexElement(0, STRUCT_OFFSET(FBlurTextureVertex, Position),	VET_Float3, 0, stride));
	elements.Add(FVertexElement(0, STRUCT_OFFSET(FBlurTextureVertex, Color),	VET_Color,	1, stride));
	VertexDeclarationRhi = RHICreateVertexDeclaration(elements);
}


void FVertexDeclarationBlur::ReleaseRHI()
{
	VertexDeclarationRhi.SafeRelease();
}


bool FVertexShaderBlur::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParams)
{
	return true;
}


FVertexShaderBlur::FVertexShaderBlur()
{
}


FVertexShaderBlur::FVertexShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& inInitializer)
	: FGlobalShader(inInitializer)
{
}


void FVertexShaderBlur::SetUniformBuffers(FRHICommandList& outRhiCmdList, FPsConstParamsBlur& outConstParams,
	FPsVarParamsBlur& outVarParams)
{
	FPsConstParamsBlurRef constParamsBuffer;
	FPsVarParamsBlurRef varParamsBuffer;

	constParamsBuffer	= FPsConstParamsBlurRef	::CreateUniformBufferImmediate(outConstParams,	UniformBuffer_SingleDraw);
	varParamsBuffer		= FPsVarParamsBlurRef	::CreateUniformBufferImmediate(outVarParams,	UniformBuffer_SingleDraw);

	SetUniformBufferParameter(outRhiCmdList, GetVertexShader(), GetUniformBufferParameter<FPsConstParamsBlur>(),	constParamsBuffer);
	SetUniformBufferParameter(outRhiCmdList, GetVertexShader(), GetUniformBufferParameter<FPsVarParamsBlur>(),		varParamsBuffer);
}


FPixelShaderBlur::FPixelShaderBlur()
{
}


bool FPixelShaderBlur::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParameters)
{
	return true;
}


bool FPixelShaderBlur::Serialize(FArchive& outArchive)
{
	bool bShaderHasOutdatedParams = FGlobalShader::Serialize(outArchive);
	outArchive << textureParameter;
	return bShaderHasOutdatedParams;
}


FPixelShaderBlur::FPixelShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& inInitializer) 
	: FGlobalShader(inInitializer)
{
	//This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
	textureParameter.Bind(inInitializer.ParameterMap, TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
}


void FPixelShaderBlur::SetUniformBuffers(FRHICommandList& outRhiCmdList,
	FPsConstParamsBlur& outConstParams,
	FPsVarParamsBlur& outVarParams)
{
	FPsConstParamsBlurRef constParamsBuffer;
	FPsVarParamsBlurRef varParamsBuffer;

	constParamsBuffer	= FPsConstParamsBlurRef	::CreateUniformBufferImmediate(outConstParams,	UniformBuffer_SingleDraw);
	varParamsBuffer		= FPsVarParamsBlurRef	::CreateUniformBufferImmediate(outVarParams,	UniformBuffer_SingleDraw);
	
	SetUniformBufferParameter(outRhiCmdList, GetPixelShader(), GetUniformBufferParameter<FPsConstParamsBlur>(), constParamsBuffer);
	SetUniformBufferParameter(outRhiCmdList, GetPixelShader(), GetUniformBufferParameter<FPsVarParamsBlur>(),	varParamsBuffer);
}


void FPixelShaderBlur::SetInputTexture(FRHICommandList& outRhiCmdList, FShaderResourceViewRHIRef inTexParamSrv)
{
	FRHIPixelShader* psRhi = GetPixelShader();

	if (textureParameter.IsBound())
	{ 
		//This actually sets the shader resource view to the texture parameter in the shader :)
		outRhiCmdList.SetShaderResourceViewParameter(psRhi, textureParameter.GetBaseIndex(), inTexParamSrv);
	}
}


void FPixelShaderBlur::UnbindBuffers(FRHICommandList& outRhiCmdList)
{
	FRHIPixelShader* psRhi = GetPixelShader();

	if (textureParameter.IsBound())
	{
		outRhiCmdList.SetShaderResourceViewParameter(psRhi, textureParameter.GetBaseIndex(), nullptr);
	}
}


//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FVertexShaderBlur,	TEXT("/GameShaders/BlurShader.usf"),TEXT("MainVertexShader"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPixelShaderBlur,	TEXT("/GameShaders/BlurShader.usf"), TEXT("MainPixelShader"), SF_Pixel);
