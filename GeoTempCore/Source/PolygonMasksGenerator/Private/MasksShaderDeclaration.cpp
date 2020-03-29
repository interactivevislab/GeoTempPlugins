#include "MasksShaderDeclaration.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParameters, "PSConstant");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParameters, "PSVariable");


void FVertexShaderExample::SetUniformBuffers(FRHICommandList& RHICmdList,
	FPixelShaderConstantParameters& ConstantParameters, FPixelShaderVariableParameters& VariableParameters)
{
	FPixelShaderConstantParametersRef ConstantParametersBuffer;
	FPixelShaderVariableParametersRef VariableParametersBuffer;

	ConstantParametersBuffer = FPixelShaderConstantParametersRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FPixelShaderVariableParametersRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);

	SetUniformBufferParameter(RHICmdList, GetVertexShader(), GetUniformBufferParameter<FPixelShaderConstantParameters>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetVertexShader(), GetUniformBufferParameter<FPixelShaderVariableParameters>(), VariableParametersBuffer);
}

FPixelShaderExample::FPixelShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {
	//This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
	TextureParameter.Bind(Initializer.ParameterMap, TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
}


void FPixelShaderExample::SetUniformBuffers(FRHICommandList& RHICmdList,
	FPixelShaderConstantParameters& ConstantParameters,
	FPixelShaderVariableParameters& VariableParameters)
{
	FPixelShaderConstantParametersRef ConstantParametersBuffer;
	FPixelShaderVariableParametersRef VariableParametersBuffer;

	ConstantParametersBuffer = FPixelShaderConstantParametersRef::CreateUniformBufferImmediate(ConstantParameters, UniformBuffer_SingleDraw);
	VariableParametersBuffer = FPixelShaderVariableParametersRef::CreateUniformBufferImmediate(VariableParameters, UniformBuffer_SingleDraw);
	
	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPixelShaderConstantParameters>(), ConstantParametersBuffer);
	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPixelShaderVariableParameters>(), VariableParametersBuffer);
}

void FPixelShaderExample::SetOutputTexture(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV)
{
	FRHIPixelShader* PixelShaderRHI = GetPixelShader();

	if (TextureParameter.IsBound()) { //This actually sets the shader resource view to the texture parameter in the shader :)
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, TextureParameter.GetBaseIndex(), TextureParameterSRV);
	}
}

void FPixelShaderExample::UnbindBuffers(FRHICommandList& RHICmdList)
{
	FRHIPixelShader* PixelShaderRHI = GetPixelShader();

	if (TextureParameter.IsBound()) {
		RHICmdList.SetShaderResourceViewParameter(PixelShaderRHI, TextureParameter.GetBaseIndex(), nullptr);
	}
}

//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FVertexShaderExample, TEXT("/GameShaders/ShadersExample.usf"), TEXT("MainVertexShader"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPixelShaderExample, TEXT("/GameShaders/ShadersExample.usf"), TEXT("MainPixelShader"), SF_Pixel);
