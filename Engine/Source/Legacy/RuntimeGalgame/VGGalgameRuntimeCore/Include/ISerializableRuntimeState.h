/*
 * ISerializableRuntimeState — 可序列化运行态片段契约（Phase 8）
 *
 * 中文：各 Runtime 子域实现本接口后由 SaveRuntime / IExecutionSnapshot 聚合；
 * 当前仅占位，具体 JSON 读写与 SaveArchive 分层在后续里程碑与 schema 版本联动完成。
 */

#pragma once
#include <NNCore/Include/File/nlohmann/json.hpp>

namespace VisionGal::GalGame
{
	struct ISerializableRuntimeState
	{
		virtual ~ISerializableRuntimeState() = default;

		virtual void SaveToJson(nlohmann::json& out) const = 0;
		virtual bool LoadFromJson(const nlohmann::json& in) = 0;
	};
}
