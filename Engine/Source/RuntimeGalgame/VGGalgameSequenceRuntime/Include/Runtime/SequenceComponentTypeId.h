/*
 * SequenceComponentTypeID — 序列剪辑组件的稳定数值类型标识
 *
 * 与 JSON 序列化字段 `type`（即 IVGSSequenceComponent::GetTypeNameID() 返回的字符串）一一对应，
 * 通过确定性哈希（FNV-1a 64）映射为 uint64，供 IVGSSequenceRuntimeSystem::SupportsType 快速匹配，
 * 避免在热路径上依赖 dynamic_cast 作为主要分派手段。
 *
 * 注意：若未来出现哈希冲突，应改为注册表分配单调 ID；当前阶段以类型名字符串为权威源。
 */
#pragma once

#include <cstdint>
#include <string_view>

#include "../../GSSExport.h"

namespace VisionGal
{
	/// 序列组件类型 ID（与 GetTypeNameID 字符串稳定对应，见 MakeSequenceComponentTypeIDFromTypeName）。
	using SequenceComponentTypeID = std::uint64_t;

	/**
	 * @brief 由类型名字符串生成 SequenceComponentTypeID（FNV-1a 64）。
	 * @param typeName 与 JSON `"type"` / GetTypeNameID() 完全一致的视图。
	 */
	[[nodiscard]] VG_GSS_API SequenceComponentTypeID MakeSequenceComponentTypeIDFromTypeName(std::string_view typeName);
}
