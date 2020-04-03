#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"


//This buffer should contain variables that never, or rarely change
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParameters, )
END_GLOBAL_SHADER_PARAMETER_STRUCT()


//This buffer is for variables that change very often (each frame for example)
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParameters, )
SHADER_PARAMETER(FVector4, Rect) //left, right, top, bottom
SHADER_PARAMETER(float, Year) //year, dummy3
END_GLOBAL_SHADER_PARAMETER_STRUCT()


typedef TUniformBufferRef<FPixelShaderConstantParameters> FPsConstParamsRef;
typedef TUniformBufferRef<FPixelShaderVariableParameters> FPsVarParamsRef;


/************************************************************************/
/* This is the type we use as vertices for our fullscreen quad.         */
/************************************************************************/
struct FMyTextureVertex
{
	FVector4 Position;
	FColor Color;
	FVector4 YearData;

	/*friend FArchive& operator<<(FArchive& Ar, FMyTextureVertex& V)
	{
		Ar << V.Position;
		Ar << V.Color;
		Ar << V.YearData;
		return Ar;
	}*/
};


/************************************************************************/
/* We define our vertex declaration to let us get our UV coords into    */
/* the shader                                                           */
/************************************************************************/
class FVertexDeclarationExample : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRhi;

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};


/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FVertexShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexShaderExample, Global);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParameters);

	FVertexShaderExample();
	FVertexShaderExample(const ShaderMetaType::CompiledShaderInitializerType& inInitializer);

	void SetUniformBuffers(FRHICommandList& outRhiCmdList, 
		FPixelShaderConstantParameters& outConstParams, FPixelShaderVariableParameters& outVarParams);
};


class FPixelShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FPixelShaderExample, Global);

public:

	FPixelShaderExample();

	explicit FPixelShaderExample(const ShaderMetaType::CompiledShaderInitializerType& inInitializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParameters);

	virtual bool Serialize(FArchive& inArchive) override;

	//This function is required to let us bind our runtime surface to the shader using an SRV.
	void SetOutputTexture(FRHICommandList& outRhiCmdList, FShaderResourceViewRHIRef inTexParamSrv);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetUniformBuffers(FRHICommandList& outRhiCmdList, 
		FPixelShaderConstantParameters& outConstParams, FPixelShaderVariableParameters& outVarParams);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void UnbindBuffers(FRHICommandList& outRhiCmdList);

private:
	//This is how you declare resources that are going to be made available in the HLSL
	FShaderResourceParameter textureParameter;
};
