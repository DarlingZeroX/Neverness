/*
 * SequenceRuntimeCommandAPI — Sequence 运行时对内唯一合法「 Side Effect 控制面」
 *
 * 设计动机：禁止 IVGSSequenceRuntimeSystem 直接修改执行器内部游标、Wait 标志或帧栈，
 * 以便将来接入断点、回放、网络锁步时能在 API 边界统一审计与拦截。
 *
 * Phase 2A：
 * - Continue / JumpToSequenceIndex 提供完整行为；
 * - PushFrame / PopFrame 为结构预留：线性播放阶段不改变栈深度（见 .cpp 空实现说明）；
 * - EmitSignal / SetVariable 为 Phase 2D / 2E 占位（无操作）。
 */
#pragma once

#include <cstddef>
#include <string_view>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	class SequenceExecutionInstance;

	/**
	 * @brief 由 SequenceExecutionInstance 作为成员持有；各系统仅通过指针调用。
	 */
	class VG_GSS_API SequenceRuntimeCommandAPI
	{
		friend class SequenceExecutionInstance;

	public:
		explicit SequenceRuntimeCommandAPI(SequenceExecutionInstance* owner);

		/// 清除活动帧的 Waiting；等价于旧 SSSequenceExecutor::Continue。
		void Continue();

		/// 将活动帧游标跳到指定索引并重置「已派发」标记；可选用于跳转类剪辑（未来 Flow）。
		void JumpToSequenceIndex(std::size_t index);

		/// 压入新帧（子序列 / 调用栈语义）；Phase 2A 不启用多帧调度，见实现注释。
		void PushFrame();

		/// 弹出当前帧；若在仅剩一帧时调用则无操作。
		void PopFrame();

		/// Phase 2D：将向 SequenceSignalBus 转发；当前无操作。
		void EmitSignal(std::string_view signalName);

		/// Phase 2E：写入 SequenceVariableTable；当前无操作。
		void SetVariable(std::string_view key, std::string_view value);

	private:
		SequenceExecutionInstance* m_Owner = nullptr;
	};
}
