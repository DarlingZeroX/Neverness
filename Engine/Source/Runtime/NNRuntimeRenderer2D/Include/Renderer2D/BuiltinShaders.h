#pragma once

/**
 * @file BuiltinShaders.h
 * @brief 硬编码 HLSL Sprite Shader（从原 GLSL 转换，逻辑完全一致）。
 *
 * 支持：Position + UV、MVP 变换、UV Rect 映射、Flip、Color Tint。
 * 注：NNRuntimeRender 的 CreateShader 实现目前只支持 HLSL，GLSL 需要转换。
 */

namespace NN::Runtime::Renderer2D::BuiltinShaders
{
    /// Sprite Vertex Shader（HLSL）
    inline constexpr const char* SpriteVS = R"(
cbuffer SpriteConstants : register(b0)
{
    float4x4 u_ViewProjection;
    float4x4 u_Transform;
    float4   u_UvRect;   // [u0, v0, u1, v1]
    float4   u_Color;    // Tint color（VS 不用，但 CB 整体绑定）
    int      u_FlipX;
    int      u_FlipY;
};

struct VSInput
{
    float3 Position : ATTRIB0;
    float2 TexCoord : ATTRIB1;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

PSInput main(VSInput input)
{
    PSInput output;
    output.Position = mul(mul(u_ViewProjection, u_Transform), float4(input.Position, 1.0));

    // UV 从 Rect 映射
    float2 uv = lerp(u_UvRect.xy, u_UvRect.zw, input.TexCoord);

    // Flip 支持
    if (u_FlipX == 1) uv.x = 1.0 - uv.x;
    if (u_FlipY == 1) uv.y = 1.0 - uv.y;

    output.TexCoord = uv;
    return output;
}
)";

    /// Sprite Fragment Shader（HLSL）
    inline constexpr const char* SpriteFS = R"(
cbuffer SpriteConstants : register(b0)
{
    float4x4 u_ViewProjection;
    float4x4 u_Transform;
    float4   u_UvRect;
    float4   u_Color;    // Tint color
    int      u_FlipX;
    int      u_FlipY;
};

// Diligent combined texture samplers 要求 sampler 命名为 {TextureName}_sampler
Texture2D    u_Texture        : register(t0);
SamplerState u_Texture_sampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
    float4 texColor = u_Texture.Sample(u_Texture_sampler, input.TexCoord);
    return texColor * u_Color;
}
)";
}
