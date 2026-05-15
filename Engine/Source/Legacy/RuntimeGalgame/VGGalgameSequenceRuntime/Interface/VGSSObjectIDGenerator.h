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

#include "../GSSExport.h"
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
	class VG_GSS_API VGSSObjectIDGenerator
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

		/**
		 * @brief 取「下一次将分配」的原始序号（用于存档；与 Generate* 共享同一计数器）。
		 * @return 当前原子计数器值，即下一次 fetch_add 将返回的 ID。
		 */
		[[nodiscard]] uint32_t GetNextRawIdForSave() const noexcept
		{
			return m_NextRawId.load(std::memory_order_acquire);
		}

		/**
		 * @brief 从存档恢复计数器（读档后调用，避免新 ID 与已持久化对象冲突）。
		 * @param nextRawIdExclusive 与 GetNextRawIdForSave 保存的值相同；必须 >= 1。
		 * @note 不恢复已注册 Gal 对象本体，仅恢复序号单调源；与 SSExecutorResourceManager 成对使用。
		 */
		void RestoreNextRawIdFromSave(uint32_t nextRawIdExclusive) noexcept
		{
			if (nextRawIdExclusive < 1u)
				nextRawIdExclusive = 1u;
			m_NextRawId.store(nextRawIdExclusive, std::memory_order_release);
		}

	private:
		uint32_t GenerateUniqueRawId() noexcept;

		/// 单调递增计数器：初始为 1，每次 fetch_add 返回刚分配到的合法 ID。
		std::atomic<uint32_t> m_NextRawId{1};
	};
}
