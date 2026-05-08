/*
 * Visual Sequence Runtime — ObjectID 生成器
 *
 * 职责：
 * - 为 Character / Sprite(Audio 纹理 ID 语义由上层解释) / Audio / Video 生成运行时唯一 ID。
 * - ID 空间统一递增，避免跨种类碰撞（便于 Timeline / Debugger 统一打印）。
 *
 * 线程安全：使用 std::atomic；每个 SSExecutorResourceManager 建议持有独立实例以隔离执行上下文。
 */
#pragma once

#include <atomic>
#include <cstdint>

#include "../VGGalScriptVisualConfig.h"
#include "VGSTypeDefine.h"

namespace VisionGal
{
	/**
	 * @brief 运行时 ObjectID 生成器（原子递增，全种类共享序号空间）。
	 *
	 * 设计说明：
	 * - 起始序号从 1 递增；0 保留为 VGSS_INVALID_OBJECT_ID。
	 * - 不在此处回收 ID（与 GC / Pool 解耦）；销毁对象不会重用旧 ID，降低悬空引用风险。
	 * - 若 uint32_t 溢出将自然回绕；调试构建可在此处加断言。
	 */
	class VG_GALGAME_VISUAL_SCRIPT_API VGSSObjectIDGenerator
	{
	public:
		VGSSObjectIDGenerator() noexcept = default;

		VGSSObjectIDGenerator(const VGSSObjectIDGenerator&) = delete;
		VGSSObjectIDGenerator& operator=(const VGSSObjectIDGenerator&) = delete;

		/// 生成角色绑定 ID（永不返回 0）。
		VGSSCharacterObjectID GenerateCharacterID() noexcept;

		/// 生成精灵（纹理对象槽位）ID。
		VGSSSpriteObjectID GenerateSpriteID() noexcept;

		/// 生成音频对象 ID。
		VGSSAudioObjectID GenerateAudioID() noexcept;

		/// 生成视频对象 ID。
		VGSSVideoObjectID GenerateVideoID() noexcept;

	private:
		uint32_t GenerateUniqueRawId() noexcept;

		/// 单调递增计数器：初始为 1，每次 fetch_add 返回刚分配到的合法 ID。
		std::atomic<uint32_t> m_NextRawId{1};
	};
}
