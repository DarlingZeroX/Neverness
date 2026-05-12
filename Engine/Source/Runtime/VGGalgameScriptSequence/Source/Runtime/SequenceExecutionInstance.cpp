/*
 * SequenceExecutionInstance — Sequence Runtime Kernel 实现
 *
 * Tick 语义与旧 SSSequenceExecutor 对齐：Playing 且绑定有效时，先对当前剪辑 Tick RuntimeSystem；
 * 若活动帧 Waiting 则本帧不再前进；否则若尚未 Execute 则查找系统并无系统则跳过；
 * Execute 后若 ShouldHoldPlaybackAfterExecute 则置 Waiting；否则本帧末尾前进一条非阻塞剪辑。
 */

#include "Runtime/SequenceExecutionInstance.h"

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

	IRuntimeInterface* SequenceExecutionInstance::QueryInterface(const InterfaceID id)
	{
		if (id == typeid(SSSequenceRuntimeDebugInfo))
		{
			m_RuntimeDebugInfo = BuildRuntimeDebugInfo();
			return &m_RuntimeDebugInfo;
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
		if (!m_Frames.empty())
		{
			m_Frames.back().Waiting = false;
			m_Frames.back().HasDispatchedCurrentClip = false;
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

	void SequenceExecutionInstance::ClearActiveFrameWaiting()
	{
		if (m_State != ESSSequenceExecutorState::Playing)
			return;

		if (!m_Frames.empty())
			m_Frames.back().Waiting = false;
	}

	void SequenceExecutionInstance::JumpActiveFrameToIndex(const std::size_t index)
	{
		if (m_Frames.empty())
			return;

		SequenceExecutionFrame& frame = m_Frames.back();
		frame.Cursor.SequenceIndex = index;
		frame.HasDispatchedCurrentClip = false;
		frame.Waiting = false;
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
		const std::size_t idx = m_Frames.back().Cursor.SequenceIndex;
		if (idx >= seq.size())
			return nullptr;

		const Ref<IVGSSequenceComponent>& ref = seq[idx];
		return ref ? ref.get() : nullptr;
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

		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;
		SequenceExecutionFrame& frame = m_Frames.back();
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
			frame.Waiting = false;
			if (idx >= seq.size())
				m_State = ESSSequenceExecutorState::Finished;
			return;
		}

		IVGSSequenceComponent* const comp = compRef.get();
		IVGSSequenceRuntimeSystem* const sys = FindRuntimeSystem(comp);
		SequenceRuntimeExecutionContext runtimeCtx = MakeRuntimeContext(m_LastDeltaTime);

		if (sys != nullptr)
			sys->Tick(comp, runtimeCtx, m_LastDeltaTime);

		if (frame.Waiting)
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
				frame.Waiting = true;
				return;
			}
		}

		++idx;
		frame.HasDispatchedCurrentClip = false;

		if (idx >= seq.size())
			m_State = ESSSequenceExecutorState::Finished;
	}

	void SequenceExecutionInstance::ProcessSignals()
	{
		// Phase 2D：处理 SequenceSignalBus 队列（异步唤醒、UI 事件合并等）。
	}

	void SequenceExecutionInstance::CleanupFinishedFrames()
	{
		// Phase 2F：并行帧组完成后在此回收；Phase 2A 无附加清理逻辑。
	}

	void SequenceExecutionInstance::EndFrame()
	{
	}

	void SequenceExecutionInstance::Tick(const float deltaTime)
	{
		BeginFrame(deltaTime);
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
			info.CurrentIndex = m_Frames.back().Cursor.SequenceIndex;
			info.Waiting = m_Frames.back().Waiting;
		}

		if (IVGSSequenceComponent* comp = GetCurrentComponentRaw())
			info.CurrentComponentType = comp->GetTypeNameID();

		return info;
	}

	std::size_t SequenceExecutionInstance::GetCurrentSequenceIndex() const noexcept
	{
		if (m_Frames.empty())
			return 0;
		return m_Frames.back().Cursor.SequenceIndex;
	}

	bool SequenceExecutionInstance::IsWaiting() const noexcept
	{
		if (m_Frames.empty())
			return false;
		return m_Frames.back().Waiting;
	}
}
