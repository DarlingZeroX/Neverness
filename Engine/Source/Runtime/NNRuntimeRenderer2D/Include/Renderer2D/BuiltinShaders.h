#pragma once

/**
 * @file BuiltinShaders.h
 * @brief 硬编码 GLSL Sprite Shader（MVP 阶段不做 Shader Asset 系统）。
 *
 * 支持：Position + UV、MVP 变换、UV Rect 映射、Flip、Color Tint。
 * 未来迁移 Diligent 时替换此文件即可。
 */

namespace NN::Runtime::Renderer2D::BuiltinShaders
{
    /// Sprite Vertex Shader（GLSL 330 core）
    inline constexpr const char* SpriteVS = R"(
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform vec4 u_UvRect;   // [u0, v0, u1, v1]
uniform int  u_FlipX;
uniform int  u_FlipY;

out vec2 v_TexCoord;

void main()
{
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);

    // UV 从 Rect 映射
    vec2 uv = mix(u_UvRect.xy, u_UvRect.zw, a_TexCoord);

    // Flip 支持
    if (u_FlipX == 1) uv.x = 1.0 - uv.x;
    if (u_FlipY == 1) uv.y = 1.0 - uv.y;

    v_TexCoord = uv;
}
)";

    /// Sprite Fragment Shader（GLSL 330 core）
    inline constexpr const char* SpriteFS = R"(
#version 330 core

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4      u_Color;  // Tint color

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = texColor * u_Color;
}
)";
}
