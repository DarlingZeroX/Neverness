/*
 * SequenceRuntimeCommandAPI — 将「控制指令」从 RuntimeSystem 与内核私有实现解耦
 */

#include "Runtime/SequenceRuntimeCommandAPI.h"

#include "Runtime/SequenceExecutionInstance.h"

namespace VisionGal::GalGame
{
	SequenceRuntimeCommandAPI::SequenceRuntimeCommandAPI(SequenceExecutionInstance* owner)
		: m_Owner(owner)
	{
	}

	void SequenceRuntimeCommandAPI::Continue()
	{
		if (m_Owner != nullptr)
			m_Owner->ClearActiveFrameWaiting();
	}

	void SequenceRuntimeCommandAPI::JumpToSequenceIndex(const std::size_t index)
	{
		if (m_Owner != nullptr)
			m_Owner->JumpActiveFrameToIndex(index);
	}

	void SequenceRuntimeCommandAPI::PushFrame()
	{
		// Phase 2A：线性 Sequence 不扩展帧栈；子序列 / 调用栈在后续 Phase 接线后，
		// 将在此处压入新 SequenceExecutionFrame 并切换数据源引用。
		(void)m_Owner;
	}

	void SequenceRuntimeCommandAPI::PopFrame()
	{
		// Phase 2A：与 PushFrame 成对保留；单帧线性播放阶段不弹出。
		(void)m_Owner;
	}

	void SequenceRuntimeCommandAPI::EmitSignal(const std::string_view signalName)
	{
		// Phase 2D：SequenceSignalBus::Emit(signalName, payload...)
		(void)signalName;
	}

	void SequenceRuntimeCommandAPI::SetVariable(const std::string_view key, const std::string_view value)
	{
		// Phase 2E：SequenceVariableTable::Set(key, Value{...})
		(void)key;
		(void)value;
	}
}
