/*
 * SSSequenceExecutor 实现 —— 纯调度，不包含具体 AVG 业务。
 */

#include "SequenceExecutor.h"

#include <utility>

#include "IVGSSequenceComponent.h"
#include "RuntimeSystem/Dialogue.h"
#include "RuntimeSystem/Background.h"
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

	SSSequenceExecutor::SSSequenceExecutor()
	{
		// 默认注册内置域；后续可由宿主 RegisterRuntimeSystem 追加或封装工厂替换。
		RegisterRuntimeSystem(std::make_unique<VGSDialogueRuntimeSystem>());
		RegisterRuntimeSystem(std::make_unique<VGSFigureRuntimeSystem>());
		RegisterRuntimeSystem(std::make_unique<VGSBackgroundRuntimeSystem>());
	}

	SSSequenceExecutor::~SSSequenceExecutor() = default;

	void SSSequenceExecutor::SetExecutionContext(SSSequenceExecutionContext* executionContext)
	{
		m_ExecutionContext = executionContext;
	}

	void SSSequenceExecutor::RegisterRuntimeSystem(std::unique_ptr<IVGSSequenceRuntimeSystem> system)
	{
		if (!system)
			return;
		m_RuntimeSystems.push_back(std::move(system));
	}

	IRuntimeInterface* SSSequenceExecutor::QueryInterface(InterfaceID id)
	{
		if (id == typeid(SSSequenceRuntimeDebugInfo))
		{
			m_RuntimeDebugInfo = BuildRuntimeDebugInfo();
			return &m_RuntimeDebugInfo;
		}

		return nullptr;
	}

	void SSSequenceExecutor::Play()
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
			m_CurrentSequenceIndex = 0;
			m_HasDispatchedCurrentClip = false;
			m_PlaybackWaiting = false;
		}

		m_State = ESSSequenceExecutorState::Playing;
	}

	void SSSequenceExecutor::Pause()
	{
		if (m_State == ESSSequenceExecutorState::Playing)
			m_State = ESSSequenceExecutorState::Paused;
	}

	void SSSequenceExecutor::Resume()
	{
		if (m_State == ESSSequenceExecutorState::Paused)
			m_State = ESSSequenceExecutorState::Playing;
	}

	void SSSequenceExecutor::Stop()
	{
		m_State = ESSSequenceExecutorState::Stopped;
		m_PlaybackWaiting = false;
		m_HasDispatchedCurrentClip = false;
	}

	void SSSequenceExecutor::Restart()
	{
		if (!HasValidSequenceBinding(m_ExecutionContext))
			return;

		m_CurrentSequenceIndex = 0;
		m_HasDispatchedCurrentClip = false;
		m_PlaybackWaiting = false;
		m_State = ESSSequenceExecutorState::Playing;
	}

	void SSSequenceExecutor::Continue()
	{
		if (m_State != ESSSequenceExecutorState::Playing)
			return;

		m_PlaybackWaiting = false;
	}

	SSSequenceExecutionContext SSSequenceExecutor::BuildExecutionContext() const
	{
		if (m_ExecutionContext != nullptr)
			return *m_ExecutionContext;

		return {};
	}

	IVGSSequenceRuntimeSystem* SSSequenceExecutor::FindRuntimeSystem(IVGSSequenceComponent* component) const
	{
		if (component == nullptr)
			return nullptr;

		// 逆序：允许宿主后注册的系统覆盖内置匹配（热补丁 / Mod / Editor 实验轨道）。
		for (auto it = m_RuntimeSystems.rbegin(); it != m_RuntimeSystems.rend(); ++it)
		{
			IVGSSequenceRuntimeSystem* const sys = it->get();
			if (sys != nullptr && sys->CanExecute(component))
				return sys;
		}

		return nullptr;
	}

	IVGSSequenceComponent* SSSequenceExecutor::GetCurrentComponentRaw() const
	{
		if (!HasValidSequenceBinding(m_ExecutionContext))
			return nullptr;

		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;
		if (m_CurrentSequenceIndex >= seq.size())
			return nullptr;

		const Ref<IVGSSequenceComponent>& ref = seq[m_CurrentSequenceIndex];
		return ref ? ref.get() : nullptr;
	}

	void SSSequenceExecutor::Tick(float deltaTime)
	{
		if (m_State != ESSSequenceExecutorState::Playing)
			return;

		if (!HasValidSequenceBinding(m_ExecutionContext))
			return;

		const auto& seq = m_ExecutionContext->SequenceData->m_Sequence;

		if (m_CurrentSequenceIndex >= seq.size())
		{
			m_State = ESSSequenceExecutorState::Finished;
			return;
		}

		const Ref<IVGSSequenceComponent>& compRef = seq[m_CurrentSequenceIndex];
		if (!compRef)
		{
			++m_CurrentSequenceIndex;
			m_HasDispatchedCurrentClip = false;
			m_PlaybackWaiting = false;
			if (m_CurrentSequenceIndex >= seq.size())
				m_State = ESSSequenceExecutorState::Finished;
			return;
		}

		IVGSSequenceComponent* const comp = compRef.get();
		IVGSSequenceRuntimeSystem* const sys = FindRuntimeSystem(comp);
		SSSequenceExecutionContext ctx = BuildExecutionContext();

		if (sys != nullptr)
			sys->Tick(comp, ctx, deltaTime);

		if (m_PlaybackWaiting)
			return;

		if (!m_HasDispatchedCurrentClip)
		{
			if (sys == nullptr)
			{
				// 未识别组件：不阻塞播放轴；跳过非法剪辑占位。
				++m_CurrentSequenceIndex;
				m_HasDispatchedCurrentClip = false;
				if (m_CurrentSequenceIndex >= seq.size())
					m_State = ESSSequenceExecutorState::Finished;
				return;
			}

			sys->Execute(comp, ctx);
			m_HasDispatchedCurrentClip = true;

			if (sys->ShouldHoldPlaybackAfterExecute(comp))
			{
				m_PlaybackWaiting = true;
				return;
			}
		}

		++m_CurrentSequenceIndex;
		m_HasDispatchedCurrentClip = false;

		if (m_CurrentSequenceIndex >= seq.size())
			m_State = ESSSequenceExecutorState::Finished;
	}

	SSSequenceRuntimeDebugInfo SSSequenceExecutor::BuildRuntimeDebugInfo() const
	{
		SSSequenceRuntimeDebugInfo info;
		info.CurrentIndex = m_CurrentSequenceIndex;
		info.Waiting = m_PlaybackWaiting;

		if (IVGSSequenceComponent* comp = GetCurrentComponentRaw())
			info.CurrentComponentType = comp->GetTypeNameID();

		return info;
	}
}
