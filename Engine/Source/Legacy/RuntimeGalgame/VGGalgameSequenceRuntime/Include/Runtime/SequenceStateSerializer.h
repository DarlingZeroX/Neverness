/*
 * SequenceStateSerializer — SequenceExecutionInstance 与 JSON 快照互转
 *
 * 不负责序列化 VGSSequenceDataContainer（剪辑表仍由资产管线管理）；仅恢复执行内核状态。
 * 通过 friend 访问 SequenceExecutionInstance 私有成员，避免过度公开内核。
 */
#pragma once

#include <NNCore/Include/File/nlohmann/json.hpp>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	class SequenceExecutionInstance;

	class VG_GSS_API SequenceStateSerializer
	{
	public:
		[[nodiscard]] static nlohmann::json Save(const SequenceExecutionInstance& instance);

		/// 将 data 应用到 instance；失败时尽力保持 instance 不变（不保证强异常安全）。
		static void Load(SequenceExecutionInstance& instance, const nlohmann::json& data);
	};
}
