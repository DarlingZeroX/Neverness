#pragma once
// =============================================================================
// RmlDiligent_Shaders.h
// 从 RmlUi DX12 Backend 移植的 HLSL Shader 源码
// 使用 Diligent 内置编译器编译为各后端格式
// =============================================================================

namespace RmlDiligent {
namespace Shaders {

// 预处理器宏（与 DX12 Backend 保持一致）
constexpr int MAX_NUM_STOPS = 16;
constexpr int BLUR_SIZE = 7;
constexpr int BLUR_NUM_WEIGHTS = (BLUR_SIZE + 1) / 2;
constexpr int MAX_NUM_STOPS_PACKED = (MAX_NUM_STOPS + 3) / 4;

// =============================================================================
// VS_Main — 主 Vertex Shader
// CB: ConstantBuffer (72 bytes: transform + translate)
// =============================================================================
static constexpr const char VS_Main[] = R"(
struct VS_INPUT
{
    float2 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer ConstantBuffer
{
    float4x4 m_transform;
    float2 m_translate;
};

cbuffer ProjectionBuffer
{
    float4x4 m_projection;
};

PS_OUTPUT main(const VS_INPUT IN)
{
    PS_OUTPUT OUT;

    // 与 GL3 一致：Projection * (Transform * (pos + translate))
    // GL3: vec2 translatedPos = inPosition + _translate;
    //      gl_Position = _transform * vec4(translatedPos, 0.0, 1.0);
    // translate 在元素本地空间（CSS transform 之前）
    float2 translatedPos = IN.position + m_translate;
    float4 resPos = mul(m_projection, mul(m_transform, float4(translatedPos.x, translatedPos.y, 0.0, 1.0)));

    OUT.position = resPos;
    OUT.color = IN.color;
    OUT.uv = IN.uv;

    return OUT;
};
)";

// =============================================================================
// VS_PassThrough — 直通 Vertex Shader（全屏四边形，NDC 坐标 + 直通 UV）
// 与 GL3 passthrough 一致：Layer/Postprocess RT 已是 RmlUi 坐标，不在此处翻转 UV。
// CB: 无
// =============================================================================
static constexpr const char VS_PassThrough[] = R"(
struct VS_INPUT
{
    float2 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

PS_OUTPUT main(const VS_INPUT IN)
{
    PS_OUTPUT OUT;
    OUT.position = float4(IN.position.x, IN.position.y, 0.0f, 1.0f);
    OUT.color = IN.color;
    OUT.uv = IN.uv;
    return OUT;
}
)";

// =============================================================================
// VS_Blur — Blur 专用 Vertex Shader（预计算 UV 偏移）
// CB: SharedConstantBuffer (112 bytes)
// =============================================================================
static constexpr const char VS_Blur[] = R"(
#define BLUR_SIZE 7
#define BLUR_NUM_WEIGHTS 4

struct VS_INPUT
{
    float2 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv[BLUR_SIZE] : TEXCOORD;
};

cbuffer SharedConstantBuffer : register(b0)
{
    float4x4 m_transform;
    float2 m_translate;
    float2 m_texelOffset;
    float4 m_weights;
    float2 m_texCoordMin;
    float2 m_texCoordMax;
};

PS_INPUT main(const VS_INPUT IN)
{
    PS_INPUT result = (PS_INPUT)0;

    for (int i = 0; i < BLUR_SIZE; i++) {
        result.uv[i] = IN.uv - float(i - BLUR_NUM_WEIGHTS + 1) * m_texelOffset;
    }
    result.position = float4(IN.position.xy, 0.0, 1.0);

    return result;
};
)";

// =============================================================================
// PS_Color — 纯颜色 Pixel Shader
// CB: 无
// =============================================================================
static constexpr const char PS_Color[] = R"(
struct VS_INPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(const VS_INPUT IN) : SV_TARGET
{
    return IN.color;
}
)";

// =============================================================================
// PS_Texture — 纹理 × 颜色 Pixel Shader
// CB: 无（使用 Texture2D + SamplerState）
// =============================================================================
static constexpr const char PS_Texture[] = R"(
struct VS_INPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

Texture2D g_InputTexture;
SamplerState g_SamplerLinear;

float4 main(const VS_INPUT IN) : SV_TARGET
{
    return IN.color * g_InputTexture.Sample(g_SamplerLinear, IN.uv);
}
)";

// =============================================================================
// PS_Passthrough — 纹理直通 Pixel Shader
// CB: 无（使用 Texture2D + SamplerState）
// =============================================================================
static constexpr const char PS_Passthrough[] = R"(
struct PS_INPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

Texture2D g_InputTexture : register(t0);
SamplerState g_SamplerLinear : register(s0);

float4 main(const PS_INPUT inputArgs) : SV_TARGET
{
    return g_InputTexture.Sample(g_SamplerLinear, inputArgs.uv);
}
)";

// =============================================================================
// PS_Blur — 7-Tap 高斯模糊 Pixel Shader
// CB: SharedConstantBuffer (112 bytes)
// =============================================================================
static constexpr const char PS_Blur[] = R"(
#define BLUR_SIZE 7
#define BLUR_NUM_WEIGHTS 4

Texture2D g_InputTexture : register(t0);
SamplerState g_SamplerLinear : register(s0);

cbuffer SharedConstantBuffer : register(b0)
{
    float4x4 m_transform;
    float2 m_translate;
    float2 m_texelOffset;
    float4 m_weights;
    float2 m_texCoordMin;
    float2 m_texCoordMax;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv[BLUR_SIZE] : TEXCOORD;
};

float4 main(const PS_INPUT IN) : SV_TARGET
{
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    for(int i = 0; i < BLUR_SIZE; i++)
    {
        float2 in_region = step(m_texCoordMin, IN.uv[i]) * step(IN.uv[i], m_texCoordMax);
        color += g_InputTexture.Sample(g_SamplerLinear, IN.uv[i]) * in_region.x * in_region.y * m_weights[abs(i - BLUR_NUM_WEIGHTS + 1)];
    }
    return color;
};
)";

// =============================================================================
// PS_DropShadow — 阴影 Pixel Shader
// CB: DropShadowBuffer (32 bytes)
// =============================================================================
static constexpr const char PS_DropShadow[] = R"(
Texture2D g_InputTexture : register(t0);
SamplerState g_SamplerLinear : register(s0);

cbuffer DropShadowBuffer : register(b0)
{
    float2 m_texCoordMin;
    float2 m_texCoordMax;
    float4 m_color;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(const PS_INPUT IN) : SV_TARGET
{
    float2 in_region = step(m_texCoordMin, IN.uv) * step(IN.uv, m_texCoordMax);
    return g_InputTexture.Sample(g_SamplerLinear, IN.uv).a * in_region.x * in_region.y * m_color;
};
)";

// =============================================================================
// PS_ColorMatrix — 色彩矩阵变换 Pixel Shader
// CB: ConstantBuffer (64 bytes)
// =============================================================================
static constexpr const char PS_ColorMatrix[] = R"(
Texture2D g_InputTexture : register(t0);
SamplerState g_SamplerLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    float4x4 m_color_matrix;
};

struct PS_Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(const PS_Input IN) : SV_TARGET
{
    float4 texColor = g_InputTexture.Sample(g_SamplerLinear, IN.uv);
    float3 transformedColor = mul(m_color_matrix, texColor).rgb;
    return float4(transformedColor, texColor.a);
};
)";

// =============================================================================
// PS_BlendMask — 双纹理混合 Pixel Shader
// CB: 无（使用 2x Texture2D + SamplerState）
// =============================================================================
static constexpr const char PS_BlendMask[] = R"(
Texture2D g_InputTexture : register(t0);
Texture2D g_MaskTexture : register(t1);
SamplerState g_SamplerLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(const PS_INPUT IN) : SV_TARGET
{
    float4 texColor = g_InputTexture.Sample(g_SamplerLinear, IN.uv);
    float maskAlpha = g_MaskTexture.Sample(g_SamplerLinear, IN.uv).a;
    return texColor * maskAlpha;
};
)";

// =============================================================================
// PS_Gradient — 渐变 Pixel Shader（线性/径向/锥形/重复）
// CB: SharedConstantBuffer (416 bytes)
// =============================================================================
static constexpr const char PS_Gradient[] = R"(
#define MAX_NUM_STOPS 16
#define MAX_NUM_STOPS_PACKED 4
#define LINEAR 0
#define RADIAL 1
#define CONIC 2
#define REPEATING_LINEAR 3
#define REPEATING_RADIAL 4
#define REPEATING_CONIC 5
#define PI 3.14159265

cbuffer SharedConstantBuffer
{
    float4x4 m_transform;
    float2 m_translate;
    int m_func;
    int m_num_stops;
    float2 m_p;
    float2 m_v;
    float4 m_stop_colors[MAX_NUM_STOPS];
    float4 m_stop_positions[MAX_NUM_STOPS_PACKED];
};

#define GET_STOP_POS(i) (m_stop_positions[i >> 2][i & 3])

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

#define glsl_mod(x,y) (((x)-(y)*floor((x)/(y))))

float4 lerp_stop_colors(float t) {
    float4 color = m_stop_colors[0];
    for (int i = 1; i < m_num_stops; i++)
        color = lerp(color, m_stop_colors[i], smoothstep(GET_STOP_POS(i-1), GET_STOP_POS(i), t));
    return color;
};

float4 main(const PS_INPUT IN) : SV_TARGET
{
    float t = 0.0;

    if (m_func == LINEAR || m_func == REPEATING_LINEAR) {
        float dist_square = dot(m_v, m_v);
        float2 V = IN.uv.xy - m_p;
        t = dot(m_v, V) / dist_square;
    }
    else if (m_func == RADIAL || m_func == REPEATING_RADIAL) {
        float2 V = IN.uv.xy - m_p;
        t = length(m_v * V);
    }
    else if (m_func == CONIC || m_func == REPEATING_CONIC) {
        float2x2 R = float2x2(m_v.x, -m_v.y, m_v.y, m_v.x);
        float2 V = mul((IN.uv.xy - m_p), R);
        t = 0.5 + atan2(-V.x, V.y) / (2.0 * PI);
    }

    if (m_func == REPEATING_LINEAR || m_func == REPEATING_RADIAL || m_func == REPEATING_CONIC) {
        float t0 = GET_STOP_POS(0);
        float t1 = GET_STOP_POS(m_num_stops - 1);
        t = t0 + glsl_mod(t - t0, t1 - t0);
    }

    return IN.color * lerp_stop_colors(t);
};
)";

// =============================================================================
// PS_Creation — 程序化效果 Pixel Shader
// CB: SharedConstantBuffer (96 bytes)
// =============================================================================
static constexpr const char PS_Creation[] = R"(
struct PS_Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer SharedConstantBuffer
{
    float4x4 m_transform;
    float2 m_translate;
    float2 m_dimensions;
    float m_value;
};

#define glsl_mod(x,y) (((x)-(y)*floor((x)/(y))))

float4 main(const PS_Input IN) : SV_TARGET
{
    float t = m_value;
    float3 c;
    float l;
    for (int i = 0; i < 3; i++) {
        float2 p = IN.uv;
        float2 uv = p;
        p -= .5;
        p.x *= m_dimensions.x / m_dimensions.y;
        float z = t + ((float)i) * .07;
        l = length(p);
        uv += p / l * (sin(z) + 1.) * abs(sin(l * 9. - z - z));
        c[i] = .01 / length(glsl_mod(uv, 1.) - .5);
    }
    return float4(c / l, IN.color.a);
};
)";

} // namespace Shaders
} // namespace RmlDiligent
