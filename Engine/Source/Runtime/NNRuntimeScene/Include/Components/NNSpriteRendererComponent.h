#pragma once

/**
 * @file NNSpriteRendererComponent.h
 * @brief 精灵渲染组件（POD，无虚表）：Render Data Component。
 *
 * 这不是"贴图+颜色"的简单组件，而是面向 Renderer 的渲染提交数据描述。
 * 设计参考：Unity SpriteRenderer、Godot CanvasItem、UE Paper2D SpriteComponent。
 *
 * 约束：
 * - trivially_copyable + standard_layout（可直接 memcpy）
 * - 无 std::string / shared_ptr / 虚函数
 * - 资源引用使用 128-bit NNGuid（永久稳定资产标识，非直接对象引用）
 * - 字段布局对 GPU Instancing 友好
 * - 所有字段可通过 NNComponentRegistry 反射自动编辑
 */

#include <cstdint>
#include <type_traits>
#include "../../NNRuntimeSceneExport.h"
#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 精灵混合模式枚举。
	 * 控制源像素与目标像素的混合方式。
	 */
	enum class NNBlendMode : std::uint32_t
	{
		Alpha    = 0,   ///< 标准 Alpha 混合 (SrcAlpha, OneMinusSrcAlpha)
		Additive = 1,   ///< 加法混合 (SrcAlpha, One)
		Multiply = 2,   ///< 正片叠底
		Opaque   = 3,   ///< 不透明（关闭混合）
		Premultiplied = 4, ///< 预乘 Alpha
	};

	/**
	 * @brief 精灵渲染标志位（可组合位掩码）。
	 */
	enum class NNSpriteFlags : std::uint32_t
	{
		None          = 0,
		Visible       = 1u << 0,  ///< 是否可见（false 时不提交渲染）
		FlipX         = 1u << 1,  ///< 水平翻转
		FlipY         = 1u << 2,  ///< 垂直翻转
		CastShadow    = 1u << 3,  ///< 投射阴影（2D Light 预留）
		ReceiveShadow = 1u << 4,  ///< 接收阴影（预留）
		Instanced     = 1u << 5,  ///< 允许 GPU Instancing
		CustomShader  = 1u << 6,  ///< 使用自定义 Shader（MaterialHandle 有效）
	};

	inline NNSpriteFlags operator|(NNSpriteFlags a, NNSpriteFlags b) noexcept
	{
		return static_cast<NNSpriteFlags>(
			static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	inline NNSpriteFlags operator&(NNSpriteFlags a, NNSpriteFlags b) noexcept
	{
		return static_cast<NNSpriteFlags>(
			static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	inline bool HasFlag(NNSpriteFlags flags, NNSpriteFlags test) noexcept
	{
		return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) != 0;
	}

	/**
	 * @brief 精灵渲染组件：纯数据描述，行为由 SpriteRenderSystem 驱动。
	 *
	 * 内存布局（88 字节）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ TextureAsset    NNGuid    16B   纹理资源 128-bit GUID │
	 * │ MaterialAsset   NNGuid    16B   材质资源 128-bit GUID │
	 * │ TextureRuntimeId uint32_t  4B   已解析的 GL Texture ID│
	 * │ (padding)                 4B                          │
	 * │ Color           float[4] 16B   RGBA tint 颜色        │
	 * │ UvRect          float[4] 16B   UV 区域 [u0,v0,u1,v1] │
	 * │ Layer           uint32_t  4B   渲染层级               │
	 * │ SortOrder       uint32_t  4B   层级内排序             │
	 * │ BlendMode       uint32_t  4B   混合模式枚举           │
	 * │ Flags           uint32_t  4B   标志位掩码             │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：8 字节，共 88 字节（22 × 4B）
	 *
	 * GPU Instancing：相邻实例 stride = 88B。
	 * SIMD 友好：Color 和 UvRect 各 16B，对齐到 16 字节边界。
	 */
	struct NN_RUNTIME_SCENE_API NNSpriteRendererComponent
	{
		// ── 资源引用（128-bit 永久稳定 GUID，序列化层）──
		// 值为全零表示未设置 / 使用默认资源。
		// 序列化时作为 Guid（high:low）持久化；Renderer 通过 Asset System 解析为 GPU 资源。
		NNGuid TextureAsset{};   ///< 纹理资源 128-bit GUID
		NNGuid MaterialAsset{};  ///< 材质资源 128-bit GUID（全零 = 使用引擎默认 SpriteShader）

		// ── Runtime 瞬态数据（不序列化）──
		std::uint32_t TextureRuntimeId = 0u; ///< 已解析的 GL Texture ID（Renderer 直接使用）

		// ── 颜色与 UV ──
		float Color[4] = {1.0f, 1.0f, 1.0f, 1.0f};  ///< Tint 颜色 RGBA [0,1]，与纹理颜色相乘
		float UvRect[4] = {0.0f, 0.0f, 1.0f, 1.0f};  ///< UV 区域 [u0, v0, u1, v1]，支持 Atlas

		// ── 渲染排序与状态 ──
		std::uint32_t Layer     = 0u;   ///< 渲染层级（类似 Unity Sorting Layer 的 ID）
		std::uint32_t SortOrder = 0u;   ///< 同 Layer 内的排序（值大的后渲染，覆盖值小的）
		NNBlendMode   Blend     = NNBlendMode::Alpha; ///< 混合模式
		NNSpriteFlags Flags     = NNSpriteFlags::Visible; ///< 标志位（默认可见）

		NNSpriteRendererComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNSpriteRendererComponent>,
		"NNSpriteRendererComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNSpriteRendererComponent>,
		"NNSpriteRendererComponent must have standard layout");
	static_assert(sizeof(NNSpriteRendererComponent) >= 80,
		"NNSpriteRendererComponent should be >= 80 bytes (NNGuid asset fields)");
} // namespace NN::Runtime::Scene
