/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzeroox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "RenderPipeline.h"
#include "ArchiveSystem.h"
#include "DialogueSystem/DialogueSystem.h"
#include "SceneSystem/LayeredSceneSystem.h"
#include "ScriptSystem/StoryScriptSystem.h"
#include "UISystem/GalUISystem.h"
#include "ResourceSystem.h"
#include "VGEngine/Include/Scene/Scene.h"
#include "VGEngine/Include/Render/RenderCore.h"
#include "VGCore/Include/Utils/TransitionHelper.h"
#include "GalSubsystemBus.h"
#include "Runtime/GalRuntimeSessionHost.h"
#include "Runtime/GalRuntimeCoordinator.h"
#include "Runtime/GalGameRuntimeHost.h"
#include <memory>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API GalGameEngine : public IGalGameEngine
	{
	public:
		GalGameEngine();
		~GalGameEngine() override;
		GalGameEngine(const GalGameEngine&) = delete;
		GalGameEngine& operator=(const GalGameEngine&) = delete;
		GalGameEngine(GalGameEngine&&) = delete;
		GalGameEngine& operator=(GalGameEngine&&) = delete;

		void Initialize(IGameEngineContext* context);
		void OnRender() override;
		void OnUpdate(float deltaTime) override;
		void Reset() override;

		ISubsystemBus* GetSubsystemBus() override;
		IGalGameContext* GetContext() override;
		IGalRuntimeSession* GetRuntimeSession() noexcept override;
		IGalGameRuntime* GetRuntime() noexcept override;

		/// 中文：Phase 8A 运行阶段；供调试器 / 子系统只读查询，**勿**用于跨线程同步原语。
		GalRuntimePhase GetRuntimePhase() const noexcept { return m_RuntimeCoordinator.GetPhase(); }

		/// 中文：协程式等待；由 IPlaybackSubsystem / IScriptSubsystem::Wait 委托。
		void WaitForStoryScript(float duration);

		/// 中文：供 IGalGameRuntime 装配使用，不延长子系统生命周期。
		IStoryScriptSystem* GetStoryScriptSystemPtr() noexcept { return m_StoryScriptSystem.get(); }
		IArchiveSystem* GetArchiveSystemPtr() noexcept { return m_ArchiveSystem.get(); }

	private:
		friend class GalSceneSubsystemAdapter;
		friend class GalAudioSubsystemAdapter;
		friend class GalUISubsystemAdapter;
		friend class GalScriptSubsystemAdapter;
		friend class GalPlaybackSubsystemAdapter;
		friend class GalArchiveSubsystemAdapter;
		friend class GalDialogueSubsystemAdapter;
		friend struct GalRuntimeSessionHost;
		friend class GalRuntimeCoordinator;

		bool TransitionCommand(const String& layer, const String& cmd);
		bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd);

		void CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext);
		void OnMainSceneChanged(const EngineEvent& evt);
		void OneRenderSceneCallback(OpenGL::RenderTarget2D* rt);

	private:
		IGameEngineContext* m_EngineContext = nullptr;
		Ref<GalGameContext> m_GalGameContext;
		Scene* m_Scene = nullptr;
		bool m_IsEngineEnable = true;

		Ref<ArchiveSystem> m_ArchiveSystem;
		Ref<DialogueSystem> m_DialogueSystem;
		Ref<LayeredSceneSystem> m_LayeredSceneManager;
		Ref<RenderPipeline> m_RenderPipeline;
		Ref<StoryScriptSystem> m_StoryScriptSystem;
		Ref<ResourceSystem> m_ResourceSystem;
		Ref<GalGameUISystem> m_GalGameUISystem;

		GalSubsystemBus m_SubsystemBus;
		GalGameRuntimeHost m_RuntimeHost;
		GalRuntimeCoordinator m_RuntimeCoordinator;

		std::unique_ptr<GalRuntimeSessionHost> m_RuntimeSession;
	};
}
