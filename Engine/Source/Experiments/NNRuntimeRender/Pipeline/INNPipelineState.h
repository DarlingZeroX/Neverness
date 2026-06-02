#pragma once
#include "../NNRenderConfig.h"
#include "../Resources/INNTexture.h"
#include "INNShader.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>
#include <vector>
namespace NN { namespace Runtime { namespace Render {
enum class NNVertexFormat : uint8_t { Float, Float2, Float3, Float4, Int, Int2, Int3, Int4, UByte4_Norm };
enum class NNVertexInputRate : uint8_t { PerVertex, PerInstance };
struct NNVertexAttribute { const char* SemanticName="POSITION"; uint32_t SemanticIndex=0; NNVertexFormat Format=NNVertexFormat::Float3; uint32_t Location=0; uint32_t Offset=0; };
struct NNVertexLayout { std::vector<NNVertexAttribute> Attributes; uint32_t Stride=0; NNVertexInputRate InputRate=NNVertexInputRate::PerVertex; };
enum class NNFillMode : uint8_t { Solid, Wireframe, Point };
enum class NNCullMode : uint8_t { None, Front, Back };
enum class NNBlendFactor : uint8_t { Zero, One, SrcAlpha, InvSrcAlpha };
enum class NNBlendOp : uint8_t { Add, Subtract, Min, Max };
enum class NNCompareFunc : uint8_t { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
struct NNRasterizerState { NNFillMode FillMode=NNFillMode::Solid; NNCullMode CullMode=NNCullMode::Back; };
struct NNBlendState { bool Enable=false; NNBlendFactor SrcBlend=NNBlendFactor::One; NNBlendFactor DestBlend=NNBlendFactor::Zero; NNBlendOp BlendOp=NNBlendOp::Add; };
struct NNDepthStencilState { bool DepthEnable=true; bool DepthWriteEnable=true; NNCompareFunc DepthFunc=NNCompareFunc::LessEqual; };
enum class NNPrimitiveTopology : uint8_t { TriangleList, TriangleStrip, LineList, LineStrip, PointList };
struct NNPipelineStateDesc {
    INNShader* VS=nullptr; INNShader* PS=nullptr;
    NNVertexLayout VertexLayout; NNRasterizerState RasterizerState; NNBlendState BlendState; NNDepthStencilState DepthStencilState;
    NNPrimitiveTopology Topology=NNPrimitiveTopology::TriangleList;
    NNPixelFormat RTVFormat=NNPixelFormat::RGBA8_UNORM; NNPixelFormat DSVFormat=NNPixelFormat::D32_FLOAT;
    uint32_t SampleCount=1; const char* DebugName=nullptr;
};
class INNPipelineState : public NN::Runtime::Core::INNObject { public: virtual const NNPipelineStateDesc& GetDesc() const=0; };
}}}
