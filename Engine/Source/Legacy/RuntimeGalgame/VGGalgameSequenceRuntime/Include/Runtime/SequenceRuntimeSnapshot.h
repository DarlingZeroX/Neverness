/*
 * SequenceRuntimeSnapshot — Sequence 执行内核可序列化状态聚合（内存态）
 *
 * 用途：
 * - 与 JSON 往返（SequenceStateSerializer）；
 * - 作为「跨帧 / 读档」的中间表示，不持有引擎指针或 Gal 对象。
 *
 * 字段说明：
 * - CurrentIndex / bIsWaiting：与栈顶帧冗余的便捷镜像，便于 Inspector；权威数据在 FrameStack。
 * - UserStateBlob：扩展点，UTF-8 键值字符串，不参与调度逻辑。
 */
#pragma once

#include "SequenceExecutionCursor.h"
#include "SequenceExecutionFrame.h"
#include "SequenceVariableTable.h"
#include "../SequenceRuntimeTypes.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 一次捕获的内核快照（不含 IVGSSequenceComponent 剪辑数据本体）。
	 */
	struct VG_GSS_API SequenceRuntimeSnapshot
	{
		std::int32_t CurrentIndex = 0;
		bool bIsWaiting = false;

		SequenceExecutionCursor TopCursor{};

		std::vector<SequenceExecutionFrame> FrameStack;

		std::uint64_t GlobalTickCounter = 0;

		ESSSequenceExecutorState State = ESSSequenceExecutorState::Stopped;

		SequenceVariableTable Variables;

		/// Wait 注册表：下一 Token 序号（与 AsyncWaitRegistry 一致）。
		std::uint64_t WaitRegistryNextTokenId = 1;
		/// 读档后仍处于 Active 的 token id（未解析集合）。
		std::vector<std::uint64_t> WaitRegistryActiveTokens;

		/// ObjectID 生成器下一原始序号（与 VGSSObjectIDGenerator::GetNextRawIdForSave 一致）。
		std::uint32_t ObjectIdNextRaw = 1;

		std::unordered_map<std::string, std::string> UserStateBlob;
	};
}
