#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"


/** Struct for constant buffer. This buffer should contain variables that never, or rarely change */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParametersBlur, )
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/** Struct for constant buffer. This buffer is for variables that change very often (each frame for example) */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParametersBlur, )
SHADER_PARAMETER(int, Steps)
SHADER_PARAMETER(float, Sigma) 
SHADER_PARAMETER(float, Distance) 
SHADER_PARAMETER(FVector2D, Direction) 
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/** Reference type for constant buffer */
typedef TUniformBufferRef<FPixelShaderConstantParametersBlur> FPsConstParamsBlurRef;
/** Reference type for variable buffer */
typedef TUniformBufferRef<FPixelShaderVariableParametersBlur> FPsVarParamsBlurRef;


/** This is the type we use as vertices for our fullscreen quad. */
struct FBlurTextureVertex
{
	FVector4 Position;
	FColor Color;	

};


/** We define our vertex declaration to let us get our UV coords into the shader */
class FVertexDeclarationBlur : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRhi;
	/** @name Implementation of FRenderResource */
	///@{
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
	///@}
};


/** A simple passthrough vertexshader that we will use. */
class FVertexShaderBlur : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexShaderBlur, Global);

public:

	FVertexShaderBlur();	
	FVertexShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& inInitializer);

	/** Create constant and variable buffers */
	void SetUniformBuffers(FRHICommandList& outRhiCmdList, FPixelShaderConstantParametersBlur& outConstParams, FPixelShaderVariableParametersBlur& outVarParams);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParams);
};

/** Base shader class */
class FPixelShaderBlur : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FPixelShaderBlur, Global);

public:

	FPixelShaderBlur();

	explicit FPixelShaderBlur(const ShaderMetaType::CompiledShaderInitializerType& inInitializer);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParams);

	virtual bool Serialize(FArchive& outArchive) override;

	/** This function is required to let us bind our runtime surface to the shader using an SRV. */
	void SetInputTexture(FRHICommandList& outRhiCmdList, FShaderResourceViewRHIRef inTexParamSrv);
	/** This function is required to bind our constant / uniform buffers to the shader. */
	void SetUniformBuffers(FRHICommandList& outRhiCmdList,
		FPixelShaderConstantParametersBlur& outConstParams, FPixelShaderVariableParametersBlur& outVarParams);
	/** This is used to clean up the buffer binds after each invocation to let them be changed and used elsewhere if needed. */
	void UnbindBuffers(FRHICommandList& outRhiCmdList);

private:
	//This is how you declare resources that are going to be made available in the HLSL
	FShaderResourceParameter textureParameter;
};
