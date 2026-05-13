/*
 * SequenceRuntimeCommandAPI — Sequence 运行时对内唯一合法「 Side Effect 控制面」
 *
 * 设计动机：禁止 IVGSSequenceRuntimeSystem 直接修改执行器内部游标、Wait 标志或帧栈，
 * 以便将来接入断点、回放、网络锁步时能在 API 边界统一审计与拦截。
 *
 * Phase 2+：Continue / Jump / EmitSignal / SetVariable / 并行组入口 等经此门面转发至内核。
 */
#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "../../GSSExport.h"
#include "ResumeToken.h"
#include "SequenceBlockingPolicy.h"
#include "SequenceValue.h"

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

		/// 清除活动帧上全部线性/并行挂起 WaitToken（无条件继续）。
		void Continue();

		/// Phase 2C：仅解析与 ResumeToken 匹配的挂起项；不匹配则忽略。
		void ContinueWithResume(const ResumeToken& token);

		/// 将活动帧游标跳到指定索引并重置「已派发」标记；可选用于跳转类剪辑（未来 Flow）。
		void JumpToSequenceIndex(std::size_t index);

		/// 压入新帧（子序列 / 调用栈语义）；Phase 2A 不启用多帧调度，见实现注释。
		void PushFrame();

		/// 弹出当前帧；若在仅剩一帧时调用则无操作。
		void PopFrame();

		/// Phase 2D：投递信号到 SequenceSignalBus（队列，ProcessSignals 派发）。
		void EmitSignal(std::string_view signalName);

		void EmitSignal(std::string_view signalName, const SequenceValue& payload);

		/// Phase 2E：写入变量表（主 API，类型完整）。
		void SetVariable(std::string_view key, const SequenceValue& value);

		/// 便捷重载：将 UTF-8 文本存为 String 类型。
		void SetVariable(std::string_view key, std::string_view stringValue);

		/**
		 * @brief Phase 2F：进入并行剪辑组（测试 / 读档 / 上层编排入口）。
		 * @param indices 并行推进的序列下标列表。
		 * @param policy WaitAll / WaitAny。
		 * @param resumeIndex 组完成后主游标跳转位置。
		 */
		void BeginParallelClipGroup(const std::vector<std::size_t>& indices, SequenceBlockingPolicy policy,
			std::size_t resumeIndex);

	private:
		SequenceExecutionInstance* m_Owner = nullptr;
	};
}
