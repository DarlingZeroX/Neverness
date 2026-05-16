/*
 * SequenceValue — Sequence 运行时统一标量值（变量 / 信号载荷 / 存档 JSON 互通）
 *
 * 设计目标：
 * - 为 Phase 2E 变量表、Phase 2D SignalBus 载荷、Phase 2B 快照序列化提供同一表示；
 * - 纯 C++17，不依赖 Galgame 引擎具体类型。
 *
 * JSON 约定（与 SequenceStateSerializer 等共用）：
 * - 对象形如 { "type": "int"|"float"|"bool"|"string", "value": ... }；
 * - 解析失败时 TryFromJson 返回 false，输出值保持调用方原有内容。
 */
#pragma once

#include <NNCore/Include/File/nlohmann/json.hpp>

#include <string>
#include <variant>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 运行时标量类型枚举。
	 */
	enum class SequenceValueType
	{
		Int,
		Float,
		Bool,
		String
	};

	/**
	 * @brief 带类型标签的运行时值（小型 variant）。
	 */
	struct VG_GSS_API SequenceValue
	{
		SequenceValueType type = SequenceValueType::Bool;
		std::variant<int, float, bool, std::string> data = false;

		static SequenceValue MakeInt(int v);
		static SequenceValue MakeFloat(float v);
		static SequenceValue MakeBool(bool v);
		static SequenceValue MakeString(std::string v);

		/// 序列化为 JSON 子对象（非完整文档根）。
		[[nodiscard]] nlohmann::json ToJson() const;

		/// 从 JSON 子对象恢复；成功返回 true。
		[[nodiscard]] static bool TryFromJson(const nlohmann::json& j, SequenceValue& out);
	};
}
