#pragma once

/**
 * @file NNAudioSourceComponent.h
 * @brief 音频源组件（POD，无虚表）：Audio Playback Data Component。
 *
 * 设计参考：Unity AudioSource、Godot AudioStreamPlayer。
 *
 * 约束：
 * - trivially_copyable + standard_layout（可直接 memcpy）
 * - 无 std::string / shared_ptr / 虚函数
 * - 资源引用使用 128-bit NNGuid（永久稳定资产标识）
 * - RuntimePlayerId 为瞬态字段，不序列化
 */

#include <cstdint>
#include <type_traits>
#include "../../NNRuntimeSceneExport.h"
#include "../../../Engine/EngineTypes.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 音频源标志位（可组合位掩码）。
	 */
	enum class NNAudioSourceFlags : std::uint32_t
	{
		None         = 0,
		PlayOnAwake  = 1u << 0,  ///< 场景启动时自动播放
		Loop         = 1u << 1,  ///< 循环播放
		Spatial      = 1u << 2,  ///< 启用 3D 空间音频
		Mute         = 1u << 3,  ///< 静音
	};

	inline NNAudioSourceFlags operator|(NNAudioSourceFlags a, NNAudioSourceFlags b) noexcept
	{
		return static_cast<NNAudioSourceFlags>(
			static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	inline NNAudioSourceFlags operator&(NNAudioSourceFlags a, NNAudioSourceFlags b) noexcept
	{
		return static_cast<NNAudioSourceFlags>(
			static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	inline bool HasFlag(NNAudioSourceFlags flags, NNAudioSourceFlags test) noexcept
	{
		return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(test)) != 0;
	}

	/**
	 * @brief 音频源组件：纯数据描述，行为由 AudioSourceSystem 驱动。
	 *
	 * 内存布局（48 字节）：
	 * ┌──────────────────────────────────────────────────────┐
	 * │ AudioClipAsset    NNGuid    16B   音频资产 GUID       │
	 * │ RuntimePlayerId   uint32_t   4B   运行时播放器句柄    │
	 * │ Volume            float      4B   音量 [0.0, 1.0]    │
	 * │ Pitch             float      4B   音调 [0.5, 2.0]    │
	 * │ MinDistance        float      4B   3D 最小距离        │
	 * │ MaxDistance        float      4B   3D 最大距离        │
	 * │ Flags             uint32_t   4B   标志位掩码          │
	 * │ _reserved0        uint32_t   4B   保留               │
	 * │ _reserved1        uint32_t   4B   保留               │
	 * └──────────────────────────────────────────────────────┘
	 * 对齐：4 字节，共 48 字节（12 × 4B）
	 */
	struct NN_RUNTIME_SCENE_API NNAudioSourceComponent
	{
		NNGuid       AudioClipAsset{};       ///< 音频资产 128-bit GUID（全零=未设置）
		std::uint32_t RuntimePlayerId = 0u;  ///< 运行时播放器句柄（transient，不序列化）
		float        Volume = 1.0f;          ///< 音量 [0.0, 1.0]
		float        Pitch = 1.0f;           ///< 音调 [0.5, 2.0]
		float        MinDistance = 1.0f;     ///< 3D 空间音频最小距离
		float        MaxDistance = 100.0f;   ///< 3D 空间音频最大距离
		std::uint32_t Flags = 0u;            ///< NNAudioSourceFlags 标志位
		std::uint32_t _reserved0 = 0u;       ///< 保留
		std::uint32_t _reserved1 = 0u;       ///< 保留

		NNAudioSourceComponent() = default;
	};

	// ── 编译期验证 ──
	static_assert(std::is_trivially_copyable_v<NNAudioSourceComponent>,
		"NNAudioSourceComponent must be trivially copyable (memcpy-safe)");
	static_assert(std::is_standard_layout_v<NNAudioSourceComponent>,
		"NNAudioSourceComponent must have standard layout");
	static_assert(sizeof(NNAudioSourceComponent) == 48,
		"NNAudioSourceComponent must be exactly 48 bytes");

} // namespace NN::Runtime::Scene
