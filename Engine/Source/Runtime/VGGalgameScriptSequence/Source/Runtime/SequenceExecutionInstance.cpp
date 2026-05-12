/*
 * SequenceExecutionInstance — Sequence Runtime Kernel 实现
 *
 * Phase 2：线性路径使用 ActiveWaitTokenIds + AsyncWaitRegistry；可选并行组见 TickActiveFramesParallel。
 */

#include "Runtime/SequenceExecutionInstance.h"

#include <cstring>
#include <utility>

#include "IVGSSequenceComponent.h"
#include "RuntimeSystem/Background.h"
#include "RuntimeSystem/Dialogue.h"
#include "RuntimeSystem/Figure.h"

namespace VisionGal::GalGame
{
	namespace
	{
		[[nodiscard]] bool HasValidSequenceBinding(const SSSequenceExecutionContext* ctx) noexcept
		{
			return ctx != nullptr && ctx->HasSequenceBinding();
		}
	}

	SequenceExecutionInstance::SequenceExecutionInstance()
		: m_CommandApi(this)
	{
		RegisterRuntimeSystem(std::make_unique<VGSDialogueRuntimeSystem>());
		RegisterRuntimeSystem(std::make_unique<VGSFigureRuntimeSystem>());
		RegisterRuntimeSystem(std::make_unique<VGSBackgroundRuntimeSystem>());
		m_Frames.push_back(SequenceExecutionFrame{});
	}

	SequenceExecutionInstance::~SequenceExecutionInstance() = default;

	void SequenceExecutionInstance::SetExecutionContext(SSSequenceExecutionContext* executionContext)
	{
		m_ExecutionContext = executionContext;
	}

	void SequenceExecutionInstance::RegisterRuntimeSystem(std::unique_ptr<IVGSSequenceRuntimeSystem> system)
	{
		if (!system)
			return;
		m_RuntimeSystems.push_back(std::move(system));
	}

	void SequenceExecutionInstance::PushFrameTrace(const char* phase) noexcept
	{
		FrameTraceEntry e{};
		e.GlobalTick = m_GlobalTickCounter;
		e.SequenceIndex = GetCurrentSequenceIndex();
		e.Waiting = IsWaiting();
		if (phase != nullptr)
		{
			std::strncpy(e.Phase, phase, sizeof(e.Phase) - 1);
			e.Phase[sizeof(e.Phase) - 1] = '\0';
		}
		if (m_FrameTraceCount < kFrameTraceCapacity)
		{
			m_FrameTrace[m_FrameTraceCount++] = e;
		}
		else
		{
			m_FrameTrace[m_FrameTraceHead] = e;
			m_FrameTraceHead = (m_FrameTraceHead + 1u) % kFrameTraceCapacity;
		}
	}

	IRuntimeInterface* SequenceExecutionInstance::QueryInterface(const InterfaceID id)
	{
		if (id == typeid(SSSequenceRuntimeDebugInfo))
		{
			m_RuntimeDebugInfo = BuildRuntimeDebugInfo();
			return &m_RuntimeDebugInfo;
		}
		if (id == typeid(SSSequenceRuntimeInspectorInfo))
		{
			m_RuntimeInspectorInfo = {};
			m_RuntimeInspectorInfo.CurrentIndex = GetCurrentSequenceIndex();
			m_RuntimeInspectorInfo.Waiting = IsWaiting();
			m_RuntimeInspectorInfo.GlobalTickCounter = m_GlobalTickCounter;
			m_RuntimeInspectorInfo.FrameStackDepth = m_Frames.size();
			for (const SequenceExecutionFrame& fr : m_Frames)
			{
				m_RuntimeInspectorInfo.FrameCursors.push_back(fr.Cursor.SequenceIndex);
				m_RuntimeInspectorInfo.FrameWaiting.push_back(
					IsLinearWaitBlocking(fr) || IsParallelWaitBlocking(fr));
			}
			if (!m_Frames.empty() && m_Frames.back().ParallelGroup.has_value())
			{
				const SequenceParallelGroup& pg = *m_Frames.back().ParallelGroup;
				m_RuntimeInspectorInfo.InParallelGroup = true;
				switch (pg.Policy)
				{
				case SequenceBlockingPolicy::None: m_RuntimeInspectorInfo.ParallelPolicy = "None"; break;
				case SequenceBlockingPolicy::WaitAll: m_RuntimeInspectorInfo.ParallelPolicy = "WaitAll"; break;
				case SequenceBlockingPolicy::WaitAny: m_RuntimeInspectorInfo.ParallelPolicy = "WaitAny"; break;
				}
				m_RuntimeInspectorInfo.ParallelActiveIndices = pg.ActiveIndices;
			}
			if (IVGSSequenceComponent* comp = GetCurrentComponentRaw())
				m_RuntimeInspectorInfo.CurrentComponentType = comp->GetTypeNameID();

			const std::size_t traceN =
				m_FrameTraceCount < kFrameTraceCapacity ? m_FrameTraceCount : kFrameTraceCapacity;
			if (m_FrameTraceCount < kFrameTraceCapacity)
			{
				for (std::size_t i = 0; i < traceN; ++i)
				{
					const FrameTraceEntry& te = m_FrameTrace[i];
					m_RuntimeInspectorInfo.FrameTraceLines.push_back(
						std::string("tick=") + std::to_string(te.GlobalTick) + " idx=" + std::to_string(te.SequenceIndex)
						+ " wait=" + (te.Waiting ? "1" : "0") + " " + te.Phase);
				}
			}
			else
			{
				for (std::size_t i = 0; i < kFrameTraceCapacity; ++i)
				{
					const FrameTraceEntry& te = m_FrameTrace[(m_FrameTraceHead + i) % kFrameTraceCapacity];
					m_RuntimeInspectorInfo.FrameTraceLines.push_back(
						std::string("tick=") + std::to_string(te.GlobalTick) + " idx=" + std::to_string(te.SequenceIndex)
						+ " wait=" + (te.Waiting ? "1" : "0") + " " + te.Phase);
				}
			}
			return &m_RuntimeInspectorInfo;
		}

		return nullptr;
	}

	void SequenceExecutionInstance::Play()
	{
		if (!HasValidSequenceBinding(m_ExecutionContext))
			return;

		if (m_State == ESSSequenceExecutorState::Paused)
		{
			m_State = ESSSequenceExecutorState::Playing;
			return;
		}

		if (m_State == ESSSequenceExecutorState::Stopped || m_State == ESSSequenceExecutorState::Finished)
		{
			if (m_Frames.empty())
				m_Frames.push_back(SequenceExecutionFrame{});
			else
				m_Frames.back() = SequenceExecutionFrame{};
		}

		m_State = ESSSequenceExecutorState::Playing;
	}

	void SequenceExecutionInstance::Pause()
	{
		if (m_State == ESSSequenceExecutorState::Playing)
			m_State = ESSSequenceExecutorState::Paused;
	}

	void SequenceExecutionInstance::Resume()
	{
		if (m_State == ESSSequenceExecutorState::Paused)
			m_State = ESSSequenceExecutorState::Playing;
	}

	void SequenceExecutionInstance::Stop()
	{
		m_State = ESSSequenceExecutorState::Stopped;
		for (SequenceExecutionFrame& fr : m_Frames)
		{
			for (const std::uint64_t id : fr.ActiveWaitTokenIds)
				m_WaitRegistry.Resolve(id);
			fr.ActiveWaitTokenIds.clear();
			if (fr.ParallelGroup.has_value())
			{
				for (std::vector<std::uint64_t>& row : fr.ParallelGroup->SlotWaitTokens)
				{
					for (const std::uint64_t id : row)
						m_WaitRegistry.Resolve(id);
					row.clear();
				}
				fr.ParallelGroup.reset();
			}
			fr.HasDispatchedCurrentClip = false;
		}
	}

	void SequenceExecutionInstance::Restart()
	{
		if (!HasValidSequenceBinding(m_ExecutionContext))
			return;

		if (m_Frames.empty())
			m_Frames.push_back(SequenceExecutionFrame{});
		else
			m_Frames.back() = SequenceExecutionFrame{};

		m_State = ESSSequenceExecutorState::Playing;
	}

	void SequenceExecutionInstance::Continue()
	{
		m_CommandApi.Continue();
	}

	void SequenceExecutionInstance::ContinueWithResumeToken(const ResumeToken& token)
	{
		if (m_State != ESSSequenceExecutorState::Playing || m_Frames.empty())
			return;
		TryResolveWaitToken(token.TokenID);
	}

	void SequenceExecutionInstance::ClearActiveFrameWaiting()
	{
		if (m_State != ESSSequenceExecutorState::Playing)
			return;

		if (m_Frames.empty())
			return;

		SequenceExecutionFrame& frame = m_Frames.back();
		for (const std::uint64_t id : frame.ActiveWaitTokenIds)
			m_WaitRegistry.Resolve(id);
		frame.ActiveWaitTokenIds.clear();
		if (frame.ParallelGroup.has_value())
		{
			for (std::vector<std::uint64_t>& row : frame.ParallelGroup->SlotWaitTokens)
			{
				for (const std::uint64_t id : row)
					m_WaitRegistry.Resolve(id);
				row.clear();
			}
		}
	}

	void SequenceExecutionInstance::TryResolveWaitToken(const std::uint64_t tokenId) noexcept
	{
		if (tokenId == 0ull || m_Frames.empty())
			return;
		SequenceExecutionFrame& frame = m_Frames.back();
		{
			std::vector<std::uint64_t>& lin = frame.ActiveWaitTokenIds;
			for (auto it = lin.begin(); it != lin.end();)
			{
				if (*it == tokenId)
				{
					m_WaitRegistry.Resolve(tokenId);
					it = lin.erase(it);
				}
				else
					++it;
			}
		}
		if (frame.ParallelGroup.has_value())
		{
			for (std::vector<std::uint64_t>& row : frame.ParallelGroup->SlotWaitTokens)
			{
				for (auto it = row.begin(); it != row.end();)
				{
					if (*it == tokenId)
					{
						m_WaitRegistry.Resolve(tokenId);
						it = row.erase(it);
					}
					else
						++it;
				}
			}
		}
	}

	void SequenceExecutionInstance::JumpActiveFrameToIndex(const std::size_t index)
	{
		if (m_Frames.empty())
			return;

		ClearActiveFrameWaiting();
		SequenceExecutionFrame& frame = m_Frames.back();
		frame.ParallelGroup.reset();
		frame.Cursor.SequenceIndex = index;
		frame.HasDispatchedCurrentClip = false;
	}

	SSSequenceExecutionContext SequenceExecutionInstance::BuildSharedContextCopy() const
	{
		if (m_ExecutionContext != nullptr)
			return *m_ExecutionContext;

		return {};
	}

	SequenceRuntimeExecutionContext SequenceExecutionInstance::MakeRuntimeContext(const float deltaTime)
	{
		SequenceRuntimeExecutionContext out;
		out.SharedContext = m_ExecutionContext;
		out.ActiveFrame = m_Frames.empty() ? nullptr : &m_Frames.back();
		out.Instance = static_cast<IStorySequenceExecutionInstance*>(this);
		out.CommandAPI = &m_CommandApi;
		out.Variables = &m_VariableTable;
		out.SignalBus = &m_SignalBus;
		out.DeltaTime = deltaTime;
		return out;
	}

	IVGSSequenceRuntimeSystem* SequenceExecutionInstance::FindRuntimeSystem(IVGSSequenceComponent* component) const
	{
		if (component == nullptr)
			return nullptr;

		const SequenceComponentTypeID typeId = component->GetComponentTypeID();

		for (auto it = m_RuntimeSystems.rbegin(); it != m_RuntimeSystems.rend(); ++it)
		{
			IVGSSequenceRuntimeSystem* const sys = it->get();
			if (sys != nullptr && sys->SupportsType(typeId))
				return sys;
		}

		for (auto it = m_RuntimeSystems.rbegin(); it != m_RuntimeSystems.rend(); ++it)
		{
			IVGSSequenceRuntimeSystem* const sys = it->get();
			if (sys != nullptr && sys->CanExecute(component))
				return sys;
		}

		return nullptr;
	}

	IVGSSequenceComponent* SequenceExecutionInstance::GetCurrentComponentRaw() const
	{
		if (!HasValidSequenceBinding(m_ExecutionContext) || m_Frames.empty())
			return nullptr;

		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;
		const SequenceExecutionFrame& frame = m_Frames.back();
		std::size_t idx = frame.Cursor.SequenceIndex;
		if (frame.ParallelGroup.has_value() && !frame.ParallelGroup->ActiveIndices.empty())
			idx = frame.ParallelGroup->ActiveIndices.front();

		if (idx >= seq.size())
			return nullptr;

		const Ref<IVGSSequenceComponent>& ref = seq[idx];
		return ref ? ref.get() : nullptr;
	}

	bool SequenceExecutionInstance::IsLinearWaitBlocking(const SequenceExecutionFrame& frame) const noexcept
	{
		for (const std::uint64_t id : frame.ActiveWaitTokenIds)
		{
			if (!m_WaitRegistry.IsResolved(id))
				return true;
		}
		return false;
	}

	bool SequenceExecutionInstance::IsParallelWaitBlocking(const SequenceExecutionFrame& frame) const noexcept
	{
		if (!frame.ParallelGroup.has_value())
			return false;
		for (const std::vector<std::uint64_t>& row : frame.ParallelGroup->SlotWaitTokens)
		{
			for (const std::uint64_t id : row)
			{
				if (!m_WaitRegistry.IsResolved(id))
					return true;
			}
		}
		return false;
	}

	void SequenceExecutionInstance::TickActiveFramesLinear(SequenceExecutionFrame& frame)
	{
		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;
		std::size_t& idx = frame.Cursor.SequenceIndex;

		if (idx >= seq.size())
		{
			m_State = ESSSequenceExecutorState::Finished;
			return;
		}

		const Ref<IVGSSequenceComponent>& compRef = seq[idx];
		if (!compRef)
		{
			++idx;
			frame.HasDispatchedCurrentClip = false;
			for (const std::uint64_t id : frame.ActiveWaitTokenIds)
				m_WaitRegistry.Resolve(id);
			frame.ActiveWaitTokenIds.clear();
			if (idx >= seq.size())
				m_State = ESSSequenceExecutorState::Finished;
			return;
		}

		IVGSSequenceComponent* const comp = compRef.get();
		IVGSSequenceRuntimeSystem* const sys = FindRuntimeSystem(comp);
		SequenceRuntimeExecutionContext runtimeCtx = MakeRuntimeContext(m_LastDeltaTime);

		if (sys != nullptr)
			sys->Tick(comp, runtimeCtx, m_LastDeltaTime);

		if (IsLinearWaitBlocking(frame))
			return;

		if (!frame.HasDispatchedCurrentClip)
		{
			if (sys == nullptr)
			{
				++idx;
				frame.HasDispatchedCurrentClip = false;
				if (idx >= seq.size())
					m_State = ESSSequenceExecutorState::Finished;
				return;
			}

			sys->Execute(comp, runtimeCtx);
			frame.HasDispatchedCurrentClip = true;

			if (sys->ShouldHoldPlaybackAfterExecute(comp))
			{
				const WaitToken wt = m_WaitRegistry.CreateWait("HoldAfterExecute");
				frame.ActiveWaitTokenIds.push_back(wt.TokenID);
				return;
			}
		}

		++idx;
		frame.HasDispatchedCurrentClip = false;

		if (idx >= seq.size())
			m_State = ESSSequenceExecutorState::Finished;
	}

	void SequenceExecutionInstance::TickActiveFramesParallel(SequenceExecutionFrame& frame)
	{
		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;
		SequenceParallelGroup& pg = *frame.ParallelGroup;

		if (pg.SlotHasDispatched.size() != pg.ActiveIndices.size())
			pg.SlotHasDispatched.assign(pg.ActiveIndices.size(), false);
		if (pg.SlotWaitTokens.size() != pg.ActiveIndices.size())
			pg.SlotWaitTokens.assign(pg.ActiveIndices.size(), {});

		if (IsParallelWaitBlocking(frame))
			return;

		SequenceRuntimeExecutionContext runtimeCtx = MakeRuntimeContext(m_LastDeltaTime);

		auto slotFinished = [&](const std::size_t si) -> bool
		{
			if (si >= pg.SlotHasDispatched.size())
				return true;
			if (!pg.SlotHasDispatched[si])
				return false;
			for (const std::uint64_t id : pg.SlotWaitTokens[si])
			{
				if (!m_WaitRegistry.IsResolved(id))
					return false;
			}
			return true;
		};

		for (std::size_t si = 0; si < pg.ActiveIndices.size(); ++si)
		{
			const std::size_t idx = pg.ActiveIndices[si];
			if (idx >= seq.size())
			{
				pg.SlotHasDispatched[si] = true;
				continue;
			}
			const Ref<IVGSSequenceComponent>& compRef = seq[idx];
			if (!compRef)
			{
				pg.SlotHasDispatched[si] = true;
				continue;
			}
			IVGSSequenceComponent* const comp = compRef.get();
			IVGSSequenceRuntimeSystem* const sys = FindRuntimeSystem(comp);
			if (sys != nullptr)
				sys->Tick(comp, runtimeCtx, m_LastDeltaTime);

			bool slotWait = false;
			for (const std::uint64_t id : pg.SlotWaitTokens[si])
			{
				if (!m_WaitRegistry.IsResolved(id))
				{
					slotWait = true;
					break;
				}
			}
			if (slotWait)
				continue;

			if (!pg.SlotHasDispatched[si])
			{
				if (sys == nullptr)
				{
					pg.SlotHasDispatched[si] = true;
					continue;
				}
				sys->Execute(comp, runtimeCtx);
				pg.SlotHasDispatched[si] = true;
				if (sys->ShouldHoldPlaybackAfterExecute(comp))
				{
					const WaitToken wt = m_WaitRegistry.CreateWait("ParallelHold");
					pg.SlotWaitTokens[si].push_back(wt.TokenID);
				}
			}
		}

		if (pg.Policy == SequenceBlockingPolicy::WaitAny)
		{
			for (std::size_t si = 0; si < pg.ActiveIndices.size(); ++si)
			{
				if (slotFinished(si))
				{
					frame.Cursor.SequenceIndex = pg.ResumeSequenceIndex;
					frame.ParallelGroup.reset();
					frame.HasDispatchedCurrentClip = false;
					frame.ActiveWaitTokenIds.clear();
					if (frame.Cursor.SequenceIndex >= seq.size())
						m_State = ESSSequenceExecutorState::Finished;
					return;
				}
			}
			return;
		}

		if (pg.Policy == SequenceBlockingPolicy::WaitAll || pg.Policy == SequenceBlockingPolicy::None)
		{
			bool allDone = true;
			for (std::size_t si = 0; si < pg.ActiveIndices.size(); ++si)
			{
				if (!slotFinished(si))
				{
					allDone = false;
					break;
				}
			}
			if (allDone)
			{
				frame.Cursor.SequenceIndex = pg.ResumeSequenceIndex;
				frame.ParallelGroup.reset();
				frame.HasDispatchedCurrentClip = false;
				frame.ActiveWaitTokenIds.clear();
				if (frame.Cursor.SequenceIndex >= seq.size())
					m_State = ESSSequenceExecutorState::Finished;
			}
		}
	}

	void SequenceExecutionInstance::BeginFrame(const float deltaTime)
	{
		m_LastDeltaTime = deltaTime;
	}

	void SequenceExecutionInstance::TickActiveFrames()
	{
		if (m_State != ESSSequenceExecutorState::Playing)
			return;

		if (!HasValidSequenceBinding(m_ExecutionContext))
			return;

		if (m_Frames.empty())
			m_Frames.push_back(SequenceExecutionFrame{});

		SequenceExecutionFrame& frame = m_Frames.back();

		if (frame.ParallelGroup.has_value())
			TickActiveFramesParallel(frame);
		else
			TickActiveFramesLinear(frame);
	}

	void SequenceExecutionInstance::ProcessSignals()
	{
		m_SignalBus.DispatchQueued();
	}

	void SequenceExecutionInstance::CleanupFinishedFrames()
	{
	}

	void SequenceExecutionInstance::EndFrame()
	{
	}

	void SequenceExecutionInstance::Tick(const float deltaTime)
	{
		++m_GlobalTickCounter;
		BeginFrame(deltaTime);
		PushFrameTrace("Tick");
		TickActiveFrames();
		ProcessSignals();
		CleanupFinishedFrames();
		EndFrame();
	}

	SSSequenceRuntimeDebugInfo SequenceExecutionInstance::BuildRuntimeDebugInfo() const
	{
		SSSequenceRuntimeDebugInfo info;
		if (!m_Frames.empty())
		{
			info.CurrentIndex = GetCurrentSequenceIndex();
			info.Waiting = IsWaiting();
		}

		if (IVGSSequenceComponent* comp = GetCurrentComponentRaw())
			info.CurrentComponentType = comp->GetTypeNameID();

		return info;
	}

	std::size_t SequenceExecutionInstance::GetCurrentSequenceIndex() const noexcept
	{
		if (m_Frames.empty())
			return 0;
		const SequenceExecutionFrame& f = m_Frames.back();
		if (f.ParallelGroup.has_value() && !f.ParallelGroup->ActiveIndices.empty())
			return f.ParallelGroup->ActiveIndices.front();
		return f.Cursor.SequenceIndex;
	}

	bool SequenceExecutionInstance::IsWaiting() const noexcept
	{
		if (m_Frames.empty())
			return false;
		const SequenceExecutionFrame& f = m_Frames.back();
		if (IsLinearWaitBlocking(f))
			return true;
		return IsParallelWaitBlocking(f);
	}
}
