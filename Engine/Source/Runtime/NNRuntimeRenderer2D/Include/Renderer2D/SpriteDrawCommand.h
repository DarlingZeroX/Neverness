#pragma once

/**
 * @file SpriteDrawCommand.h
 * @brief 单个 Sprite 绘制命令：POD 结构，可直接 memcpy。
 *
 * 由 SpriteRenderSystem 从 ECS (Transform + SpriteRenderer) 收集生成，
 * 由 Renderer2D 消费并提交到 RHI（OpenGL）。
 * 不暴露任何 OpenGL 类型到高层。
 */

#include <cstdint>

namespace NN::Runtime::Renderer2D
{
    /// 混合模式（与 NNSpriteRendererComponent::NNBlendMode 值对齐）
    enum class BlendMode : std::uint32_t
    {
        Alpha = 0,
        Additive,
        Multiply,
        Opaque,
        Premultiplied
    };

    /// 单个 Sprite 绘制命令（64 字节对齐友好）
    struct SpriteDrawCommand
    {
        float             Transform[16];   ///< WorldMatrix（4x4，列主序，来自 TransformComponent）
        std::uint64_t     TextureHandle;   ///< RHI Texture Handle（0 = 白色默认纹理）
        float             Color[4];        ///< RGBA tint [0,1]
        float             UvRect[4];       ///< [u0, v0, u1, v1]（Atlas UV 区域）
        std::uint32_t     Layer;           ///< 渲染层（排序用）
        std::uint32_t     SortOrder;       ///< 层内排序（大的后渲染）
        BlendMode         Blend;           ///< 混合模式
        bool              FlipX;           ///< 水平翻转
        bool              FlipY;           ///< 垂直翻转
    };
}
