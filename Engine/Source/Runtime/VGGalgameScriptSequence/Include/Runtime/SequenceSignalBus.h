/*
 * SequenceSignalBus — Sequence 内核内部事件总线（单线程）
 *
 * - Emit：将信号排入队列，由 SequenceExecutionInstance::ProcessSignals 在同一帧或下一节拍消费；
 * - Subscribe：按信号名注册回调；回调内禁止再次 Emit 同名信号以防重入时，应只投递到队列（本实现已 swap 队列）。
 *
 * 线程：未加锁；跨线程 Emit/Subscribe 未定义行为。
 */
#pragma once

#include "SequenceValue.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	struct VG_GSS_API SequenceSignal
	{
		std::string Name;
		SequenceValue Payload;
	};

	class VG_GSS_API SequenceSignalBus
	{
	public:
		void Emit(SequenceSignal signal);

		void Subscribe(const std::string& name, std::function<void(const SequenceSignal&)> fn);

		/// 由内核每帧调用：分发队列中信号并清空队列。
		void DispatchQueued();

		void ClearSubscribers() noexcept;

	private:
		std::vector<SequenceSignal> m_Queue;
		std::unordered_map<std::string, std::vector<std::function<void(const SequenceSignal&)>>> m_Subscribers;
	};
}
