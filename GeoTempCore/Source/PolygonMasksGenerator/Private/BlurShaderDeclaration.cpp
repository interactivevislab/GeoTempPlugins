#include "BlurShaderDeclaration.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "RHIResources.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParametersBlur, "PSConstantB");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParametersBlur, "PSVariableB");


void FVertexShaderBlur::SetUniformBuffers(FRHICommandList& RHICmdList,
	FPixelShaderConstantParametersBlur& ConstantParameters, FPixelShaderVariableParametersBlur& VariableParameters)
{
	FPixelShaderConstantParametersBlurRef ConstantParametersBuffer;
	FPixelShaderVariableParametersBlurRef VariableParametersBuffer;

	ConstantParametersBuffer = FPixelShaderConstantParametersBlurRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FPixelShaderVariableParametersBlurRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, GetVertexShader(), GetUniformBufferParameter<FPixelShaderConstantParametersBlur>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetVertexShader(), GetUniformBufferParameter<FPixelShaderVariableParametersBlur>(), VariableParametersBuffer);
}

FPixelShaderBlur::FPixelShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {
	//This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
	TextureParameter.Bind(Initializer.ParameterMap, TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
}


void FPixelShaderBlur::SetUniformBuffers(FRHICommandList& RHICmdList,
	FPixelShaderConstantParametersBlur& ConstantParameters,
	FPixelShaderVariableParametersBlur& VariableParameters)
{
	FPixelShaderConstantParametersBlurRef ConstantParametersBuffer;
	FPixelShaderVariableParametersBlurRef VariableParametersBuffer;

	ConstantParametersBuffer = FPixelShaderConstantParametersBlurRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FPixelShaderVariableParametersBlurRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);
	
	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPixelShaderConstantParametersBlur>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPixelShaderVariableParametersBlur>(), VariableParametersBuffer);
}

void FPixelShaderBlur::SetInputTexture(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV)
{
	FRHIPixelShader* PixelShaderRHI = GetPixelShader();

	if (TextureParameter.IsBound()) { //This actually sets the shader resource view to the texture parameter in the shader :)
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, TextureParameter.GetBaseIndex(), TextureParameterSRV);
	}
}

void FPixelShaderBlur::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FRHIPixelShader* PixelShaderRHI = GetPixelShader();

	if (TextureParameter.IsBound()) {
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, TextureParameter.GetBaseIndex(), nullptr);
	}
}

//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FVertexShaderBlur, TEXT("/GameShaders/BlurShader.usf"), TEXT("MainVertexShader"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPixelShaderBlur, TEXT("/GameShaders/BlurShader.usf"), TEXT("MainPixelShader"), SF_Pixel);
