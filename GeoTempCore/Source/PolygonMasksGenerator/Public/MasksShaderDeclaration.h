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

typedef TUniformBufferRef<FPixelShaderConstantParameters> FPixelShaderConstantParametersRef;
typedef TUniformBufferRef<FPixelShaderVariableParameters> FPixelShaderVariableParametersRef;

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
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FMyTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, Position), VET_Float3, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, Color), VET_Color, 1, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FMyTextureVertex, YearData), VET_Float4, 2, Stride));
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
class FVertexShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexShaderExample, Global);
public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	};

	FVertexShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{}
	FVertexShaderExample() {}

	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParameters& ConstantParameters, FPixelShaderVariableParameters& VariableParameters);
};


class FPixelShaderExample : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FPixelShaderExample, Global);

public:

	FPixelShaderExample() {}

	explicit FPixelShaderExample(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

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
	void SetOutputTexture(FRHICommandList& RHICmdList, FShaderResourceViewRHIRef TextureParameterSRV);
	//This function is required to bind our constant / uniform buffers to the shader.
	void SetUniformBuffers(FRHICommandList& RHICmdList, FPixelShaderConstantParameters& ConstantParameters, FPixelShaderVariableParameters& VariableParameters);
	//This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed.
	void UnbindBuffers(FRHICommandList& RHICmdList);

private:
	//This is how you declare resources that are going to be made available in the HLSL
	FShaderResourceParameter TextureParameter;
};