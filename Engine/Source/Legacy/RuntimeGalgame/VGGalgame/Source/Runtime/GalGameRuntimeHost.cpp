/*
 * GalGameRuntimeHost — IGalGameRuntime 装配实现
 */

#include "Runtime/GalGameRuntimeHost.h"
#include "GalGameEngine.h"
#include "VGGalgameCore/Interface/IGameSystem.h"
#include "VGGalgameContract/Interface/ISubsystemBus.h"

namespace VisionGal::GalGame
{
	GalGameRuntimeHost::GalGameRuntimeHost(GalGameEngine* engine) noexcept
		: m_Engine(engine)
	{
	}

	IStoryScriptSystem* GalGameRuntimeHost::GetExecutionRuntime() noexcept
	{
		return m_Engine ? m_Engine->GetStoryScriptSystemPtr() : nullptr;
	}

	IArchiveSystem* GalGameRuntimeHost::GetSaveRuntime() noexcept
	{
		return m_Engine ? m_Engine->GetArchiveSystemPtr() : nullptr;
	}

	IPlaybackSubsystem* GalGameRuntimeHost::GetPlaybackRuntime() noexcept
	{
		if (!m_Engine)
			return nullptr;
		auto* bus = m_Engine->GetSubsystemBus();
		return bus ? bus->Playback() : nullptr;
	}

	IVariableRuntime* GalGameRuntimeHost::GetVariableRuntime() noexcept
	{
		return nullptr;
	}
}
