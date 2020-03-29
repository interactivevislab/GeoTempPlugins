#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

//This buffer should contain variables that never, or rarely change
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParametersBlur, )
END_GLOBAL_SHADER_PARAMETER_STRUCT()

//This buffer is for variables that change very often (each frame for example)
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParametersBlur, )
SHADER_PARAMETER(int, Steps)
SHADER_PARAMETER(float, Sigma) 
SHADER_PARAMETER(float, Distance) 
SHADER_PARAMETER(FVector2D, Direction) 
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FPixelShaderConstantParametersBlur> FPixelShaderConstantParametersBlurRef;
typedef TUniformBufferRef<FPixelShaderVariableParametersBlur> FPixelShaderVariableParametersBlurRef;

/************************************************************************/
/* This is the type we use as vertices for our fullscreen quad.         */
/************************************************************************/
struct FBlurTextureVertex
{
	FVector4 Position;
	FColor Color;	

	/*friend FArchive& operator<<(FArchive& Ar, FBlurTextureVertex& V)
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
class FVertexDeclarationBlur : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FBlurTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FBlurTextureVertex, Position), VET_Float3, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FBlurTextureVertex, Color), VET_Color, 1, Stride));		
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FVertexShaderBlur : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexShaderBlur, Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	};

	FVertexShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{}
	FVertexShaderBlur() {}

	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParametersBlur& ConstantParameters, FPixelShaderVariableParametersBlur& VariableParameters);
};


class FPixelShaderBlur : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FPixelShaderBlur, Global);

public:

	FPixelShaderBlur() {}

	explicit FPixelShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	};

	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);

		Ar << TextureParameter;

		return bShaderHasOutdatedParams;
	}

	//This function is required to let us bind our runtime surface to the shader using an SRV.
	void SetInputTexture(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParametersBlur& ConstantParameters, FPixelShaderVariableParametersBlur& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void UnbindBuffers(FRHICommandList& RHICmdList);

private:
	//This is how you declare resources that are going to be made available in the HLSL
	FShaderResourceParameter TextureParameter;
};