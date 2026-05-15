/*
 * GalGameRuntimeHost — IGalGameRuntime 宿主实现（Phase 8 / VGGalgame）
 *
 * 中文：由 GalGameEngine 持有，将现有子系统指针装配为「多域 Runtime」视图；
 * **GetPlaybackRuntime** 与 **ISubsystemBus::Playback()** 指向同一 IPlaybackSubsystem 实现，避免重复状态。
 */

#pragma once
#include "../../VGGalgameConfig.h"
#include "VGGalgameContract/Interface/IGalGameRuntime.h"

namespace VisionGal::GalGame
{
	class GalGameEngine;

	struct VG_GALGAME_API GalGameRuntimeHost final : public IGalGameRuntime
	{
		explicit GalGameRuntimeHost(GalGameEngine* engine) noexcept;

		IStoryScriptSystem* GetExecutionRuntime() noexcept override;
		IArchiveSystem* GetSaveRuntime() noexcept override;
		IPlaybackSubsystem* GetPlaybackRuntime() noexcept override;
		IVariableRuntime* GetVariableRuntime() noexcept override;

	private:
		GalGameEngine* m_Engine = nullptr;
	};
}
