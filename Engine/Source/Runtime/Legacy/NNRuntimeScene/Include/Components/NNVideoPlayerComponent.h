#pragma once

/**
 * @file NNVideoPlayerComponent.h
 * @brief 视频播放器组件（POD，无虚表）：Video Playback Data Component。
 *
 * 设计参考：Unity VideoPlayer、Godot VideoStreamPlayer。
 *
 * 约束：
 * - trivially_copyable + standard_layout（可直接 memcpy）
 * - 无 std::string / shared_ptr / 虚函数
 * - 资源引用使用 128-bit NNGuid（永久稳定资产标识）
 * - RuntimePlayerId / VideoTextureId 为瞬态字段，不序列化
 */

#include <cstdint>
#include <type_traits>
#include "../../NNRuntimeSceneExport.h"
#include "../../../Engine/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 视频播放器标志位（可组合位掩码）。
	 */
	enum class NNVideoPlayerFlags : std::uint32_t
	{
		None            = 0,
		PlayOnAwake     = 1u << 0,  ///< 场景启动时自动播放
		Loop            = 1u << 1,  ///< 循环播放
		RenderToSprite  = 1u << 2,  ///< 渲染到 SpriteRenderer
		Mute            = 1u << 3,  ///< 静音
		Paused          = 1u << 4,  ///< 暂停状态
	};

	inline NNVideoPlayerFlags operator|(NNVideoPlayerFlags a, NNVideoPlayerFlags b) noexcept
	{
		return static_cast<NNVideoPlayerFlags>(
			static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	inline NNVideoPlayerFlags operator&(NNVideoPlayerFlags a, NNVideoPlayerFlags b) noexcept
	{
		return static_cast<NNVideoPlayerFlags>(
			static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	inline bool HasFlag(NNVideoPlayerFlags flags, NNVideoPlayerFlags test) noexcept
	{
		return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) != 0;
	}

	/**
	 * @brief 视频播放器组件：纯数据描述，行为由 VideoPlayerSystem 驱动。
	 *
	 * 内存布局（56 字节）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ VideoClipAsset    NNGuid    16B   视频资产 GUID       │
	 * │ RuntimePlayerId   uint32_t   4B   运行时播放器句柄    │
	 * │ VideoTextureId    uint32_t   4B   GPU 纹理 ID        │
	 * │ Volume            float      4B   音量 [0.0, 1.0]    │
	 * │ Flags             uint32_t   4B   标志位掩码          │
	 * │ TargetSprite      NNGuid    16B   渲染目标 Sprite GUID│
	 * │ _reserved0        uint32_t   4B   保留               │
	 * │ _reserved1        uint32_t   4B   保留               │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：4 字节，共 56 字节（14 × 4B）
	 */
	struct NN_RUNTIME_SCENE_API NNVideoPlayerComponent
	{
		NNGuid       VideoClipAsset{};       ///< 视频资产 128-bit GUID（全零=未设置）
		std::uint32_t RuntimePlayerId = 0u;  ///< 运行时播放器句柄（transient，不序列化）
		std::uint32_t VideoTextureId = 0u;   ///< GPU 纹理 ID（transient，不序列化）
		float        Volume = 1.0f;          ///< 音量 [0.0, 1.0]
		std::uint32_t Flags = 0u;            ///< NNVideoPlayerFlags 标志位
		NNGuid       TargetSprite{};         ///< 渲染目标 SpriteRenderer GUID（全零=独立渲染）
		std::uint32_t _reserved0 = 0u;       ///< 保留
		std::uint32_t _reserved1 = 0u;       ///< 保留

		NNVideoPlayerComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNVideoPlayerComponent>,
		"NNVideoPlayerComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNVideoPlayerComponent>,
		"NNVideoPlayerComponent must have standard layout");
	static_assert(sizeof(NNVideoPlayerComponent) == 56,
		"NNVideoPlayerComponent must be exactly 56 bytes");

} // namespace NN::Runtime::Scene
