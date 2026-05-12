/*
 * SequenceVariableTable — Galgame Sequence 运行时字符串键变量表
 *
 * 契约说明（重要）：
 * - Get(key)：若键不存在，返回 **Bool 类型的 false**（与「未定义」区分需在业务层用 TryGet）；
 * - TryGet(key, out)：键存在且写入 out 返回 true，否则 false，out 不变；
 * - 线程：与 SequenceExecutionInstance 相同，假定单线程游戏线程调用，无内部锁。
 *
 * 存档：变量表整体由 SequenceStateSerializer 序列化进快照 JSON（见 Phase 2B）。
 */
#pragma once

#include "SequenceValue.h"

#include <optional>
#include <string>
#include <unordered_map>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 字符串键 → SequenceValue 的扁平表。
	 */
	class VG_GSS_API SequenceVariableTable
	{
	public:
		void Set(std::string key, SequenceValue value);

		/// 键不存在时返回 MakeBool(false)，见头文件契约说明。
		[[nodiscard]] SequenceValue Get(const std::string& key) const;

		[[nodiscard]] bool TryGet(const std::string& key, SequenceValue& out) const;

		[[nodiscard]] bool Contains(const std::string& key) const noexcept;

		void Clear() noexcept;

		[[nodiscard]] const std::unordered_map<std::string, SequenceValue>& RawMap() const noexcept { return m_Table; }

	private:
		std::unordered_map<std::string, SequenceValue> m_Table;
	};
}
