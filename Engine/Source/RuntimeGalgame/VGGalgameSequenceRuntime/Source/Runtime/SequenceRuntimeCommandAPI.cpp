/*
 * SequenceRuntimeCommandAPI — 将「控制指令」从 RuntimeSystem 与内核私有实现解耦
 */

#include "Runtime/SequenceRuntimeCommandAPI.h"

#include "Runtime/SequenceExecutionInstance.h"
#include "Runtime/SequenceParallelGroup.h"
#include "Runtime/SequenceSignalBus.h"
#include "Runtime/SequenceValue.h"

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

	void SequenceRuntimeCommandAPI::ContinueWithResume(const ResumeToken& token)
	{
		if (m_Owner != nullptr)
			m_Owner->ContinueWithResumeToken(token);
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
		EmitSignal(signalName, SequenceValue::MakeString(std::string{}));
	}

	void SequenceRuntimeCommandAPI::EmitSignal(const std::string_view signalName, const SequenceValue& payload)
	{
		if (m_Owner == nullptr)
			return;
		SequenceSignal sig;
		sig.Name = std::string(signalName);
		sig.Payload = payload;
		m_Owner->m_SignalBus.Emit(std::move(sig));
	}

	void SequenceRuntimeCommandAPI::SetVariable(const std::string_view key, const SequenceValue& value)
	{
		if (m_Owner == nullptr)
			return;
		m_Owner->m_VariableTable.Set(std::string(key), value);
	}

	void SequenceRuntimeCommandAPI::SetVariable(const std::string_view key, const std::string_view stringValue)
	{
		SetVariable(key, SequenceValue::MakeString(std::string(stringValue)));
	}

	void SequenceRuntimeCommandAPI::BeginParallelClipGroup(const std::vector<std::size_t>& indices,
		const SequenceBlockingPolicy policy, const std::size_t resumeIndex)
	{
		if (m_Owner == nullptr || indices.empty())
			return;
		if (m_Owner->m_Frames.empty())
			return;
		SequenceParallelGroup pg;
		pg.ActiveIndices = indices;
		pg.Policy = policy;
		pg.ResumeSequenceIndex = resumeIndex;
		pg.SlotHasDispatched.assign(pg.ActiveIndices.size(), false);
		pg.SlotWaitTokens.assign(pg.ActiveIndices.size(), {});
		m_Owner->m_Frames.back().ParallelGroup = std::move(pg);
		m_Owner->m_Frames.back().HasDispatchedCurrentClip = false;
		m_Owner->m_Frames.back().ActiveWaitTokenIds.clear();
	}
}
