#include "MasksShaderDeclaration.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderConstantParameters, "PSConstant");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FPixelShaderVariableParameters, "PSVariable");


void FVertexDeclarationExample::InitRHI()
{
    FVertexDeclarationElementList elements;
    uint32 stride = sizeof(FMaskPolygonVertex);
    elements.Add(FVertexElement(0, STRUCT_OFFSET(FMaskPolygonVertex, Position),    VET_Float3, 0, stride));
    elements.Add(FVertexElement(0, STRUCT_OFFSET(FMaskPolygonVertex, Color),        VET_Color,    1, stride));
    elements.Add(FVertexElement(0, STRUCT_OFFSET(FMaskPolygonVertex, YearData),    VET_Float4, 2, stride));
    VertexDeclarationRhi = RHICreateVertexDeclaration(elements);
}


void FVertexDeclarationExample::ReleaseRHI()
{
    VertexDeclarationRhi.SafeRelease();
}


void FVertexShaderExample::SetUniformBuffers(FRHICommandList& outRhiCmdList, 
    FPixelShaderConstantParameters& outConstParams, FPixelShaderVariableParameters& outVarParams)
{
    FPsConstParamsRef constParamsBuffer;
    FPsVarParamsRef varParamsBuffer;

    constParamsBuffer    = FPsConstParamsRef    ::CreateUniformBufferImmediate(outConstParams,    UniformBuffer_SingleDraw);
    varParamsBuffer        = FPsVarParamsRef    ::CreateUniformBufferImmediate(outVarParams,    UniformBuffer_SingleDraw);

    SetUniformBufferParameter(outRhiCmdList, GetVertexShader(),
        GetUniformBufferParameter<FPixelShaderConstantParameters>(), constParamsBuffer);
    SetUniformBufferParameter(outRhiCmdList, GetVertexShader(),
        GetUniformBufferParameter<FPixelShaderVariableParameters>(), varParamsBuffer);
}


FVertexShaderExample::FVertexShaderExample()
{
}


FVertexShaderExample::FVertexShaderExample(const ShaderMetaType::CompiledShaderInitializerType& inInitializer)
    : FGlobalShader(inInitializer)
{
}


bool FVertexShaderExample::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParameters)
{
    return true;
}


FPixelShaderExample::FPixelShaderExample(const ShaderMetaType::CompiledShaderInitializerType& inInitializer)
    : FGlobalShader(inInitializer)
{
    //This call is what lets the shader system know that the surface OutputSurface is going to be available in the shader. The second parameter is the name it will be known by in the shader
    textureParameter.Bind(inInitializer.ParameterMap, TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
}


FPixelShaderExample::FPixelShaderExample()
{
}


bool FPixelShaderExample::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& inParameters)
{
    return true;
}


bool FPixelShaderExample::Serialize(FArchive& inArchive)
{
    bool bShaderHasOutdatedParams = FGlobalShader::Serialize(inArchive);
    inArchive << textureParameter;
    return bShaderHasOutdatedParams;
}


void FPixelShaderExample::SetUniformBuffers(FRHICommandList& outRhiCmdList, 
    FPixelShaderConstantParameters& outConstParams, FPixelShaderVariableParameters& outVarParams)
{
    FPsConstParamsRef constParamsBuffer;
    FPsVarParamsRef varParamsBuffer;

    constParamsBuffer    = FPsConstParamsRef    ::CreateUniformBufferImmediate(outConstParams,    UniformBuffer_SingleDraw);
    varParamsBuffer        = FPsVarParamsRef    ::CreateUniformBufferImmediate(outVarParams,    UniformBuffer_SingleDraw);
    
    SetUniformBufferParameter(outRhiCmdList, GetPixelShader(), 
        GetUniformBufferParameter<FPixelShaderConstantParameters>(), constParamsBuffer);
    SetUniformBufferParameter(outRhiCmdList, GetPixelShader(), 
        GetUniformBufferParameter<FPixelShaderVariableParameters>(), varParamsBuffer);
}


void FPixelShaderExample::SetOutputTexture(FRHICommandList& outRhiCmdList, FShaderResourceViewRHIRef inTexParamSrv)
{
    FRHIPixelShader* PixelShaderRHI = GetPixelShader();

    if (textureParameter.IsBound())
    {
        //This actually sets the shader resource view to the texture parameter in the shader :)
        outRhiCmdList.SetShaderResourceViewParameter(PixelShaderRHI, textureParameter.GetBaseIndex(), inTexParamSrv);
    }
}


void FPixelShaderExample::UnbindBuffers(FRHICommandList& outRhiCmdList)
{
    FRHIPixelShader* PixelShaderRHI = GetPixelShader();

    if (textureParameter.IsBound()) {
        outRhiCmdList.SetShaderResourceViewParameter(PixelShaderRHI, textureParameter.GetBaseIndex(), nullptr);
    }
}


//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FVertexShaderExample,    TEXT("/GameShaders/ShadersExample.usf"), TEXT("MainVertexShader"),    SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FPixelShaderExample,    TEXT("/GameShaders/ShadersExample.usf"), TEXT("MainPixelShader"),    SF_Pixel);
