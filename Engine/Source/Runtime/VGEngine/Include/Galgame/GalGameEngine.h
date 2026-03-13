/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "../EngineConfig.h"
#include "Interface/IGalGameEngine.h"
#include "RenderPipeline.h"
#include "ArchiveSystem.h"
#include "DialogueSystem/DialogueSystem.h"
#include "SceneSystem/LayeredSceneSystem.h"
#include "ScriptSystem/StoryScriptSystem.h"
#include "UISystem/GalUISystem.h"
#include "ResourceSystem.h"
#include "../Scene/Scene.h"
#include "../Render/RenderCore.h"
#include "../Utils/TransitionHelper.h"

namespace VisionGal::GalGame
{
	class VG_ENGINE_API GalGameEngine: public IGalGameEngine
	{
	public:
		GalGameEngine();
		~GalGameEngine() override = default;
		GalGameEngine(const GalGameEngine&) = delete;
		GalGameEngine& operator=(const GalGameEngine&) = delete;
		GalGameEngine(GalGameEngine&&) noexcept = default;
		GalGameEngine& operator=(GalGameEngine&&) noexcept = default;

		/// 预加载指定路径的资源。
		bool PreLoadResource(const String& path) override;	

		// 转场相关接口
		bool TransitionCommand(const String& layer, const String& cmd) override;	/// 执行指定图层上的转场命令。
		bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd);

		// 资源添加相关接口
		GalSprite* ShowSprite(const std::string& layer, const std::string& path) override;	/// 在指定图层上显示精灵，并返回精灵对象指针。
		GalSprite* ShowColor(const std::string& layer, const float4& color);				/// 在指定图层上显示颜色，并返回精灵对象指针。
		GalAudio* PlayAudio(const std::string& layer, const std::string& path) override;	/// 播放指定图层上的音频文件。并返回音频对象指针。
		GalVideo* PlayVideo(const std::string& layer, const std::string& path);				/// 播放指定图层上的视频文件。并返回视频对象指针。
		GalCharacter* CreateCharacter(const String& name) override;							/// 创建一个具有指定人物名称的 GalCharacter 实例
		bool LoadArchive(const SaveArchive& archive) override;								/// 加载指定的剧情存档对象

		// 资源移除相关接口
		bool RemoveSprite(GalSprite* sprite) override;	/// 移除指定的精灵对象。
		bool RemoveAudio(GalAudio* audio) override;		/// 移除指定的音频对象。
		void HideAllCharacterSprite() override;			/// 隐藏所有角色精灵。

		// 核心子系统获取接口
		IArchiveSystem* GetArchiveSystem() override;	/// 获取存档系统的指针
		IDialogueSystem* GetDialogueSystem() override;	/// 获取对话系统的指针
		ILayeredSceneManager* GetLayeredSceneManager() override;	/// 获取分层场景管理器的指针
		IStoryScriptSystem* GetStoryScriptSystem();
		GalGameUISystem* GetGalGameUISystem();

		// 剧情脚本相关接口
		void ReloadStoryScript() override;	/// 重新加载剧情脚本
		bool LoadStoryScript(const String& path) override;	/// 加载指定路径的剧情脚本
		void LoadStoryScriptOnUpdate(const String& path) override;	/// 在更新时加载指定路径的剧情脚本

		// 引擎生命周期接口
		void Initialize(IGameEngineContext* context);
		void OnRender() override;
		void OnUpdate(float deltaTime) override;
		void Reset() override;	/// 重置引擎的状态，通常用于将引擎恢复到初始状态。
		void Wait(float duration) override;	/// 等待指定的时间长度。

		// 场景图像捕获接口
		void CaptureSceneImage();

		// 引擎上下文获取接口
		ArchiveDataContainer* GetArchiveDataContainer() const;
	private:
		void CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext);		/// 创建引擎的核心子系统。
		void OnMainSceneChanged(const EngineEvent& evt);			/// 处理主场景更改事件的回调函数。
		void OneRenderSceneCallback(OpenGL::RenderTarget2D* rt);		/// 引擎渲染回调函数。
	private:
		IGameEngineContext* m_EngineContext;					// 引擎上下文，包含了引擎的各种系统和状态。
		Ref<GalGameContext> m_GalGameContext;				// 引擎上下文
		Scene* m_Scene;											// 当前的场景对象，表示游戏中的一个具体场景。
		bool m_IsEngineEnable = true;							// 引擎是否启用的标志，指示引擎是否处于活动状态。

		// 引擎核心子系统
		Ref<ArchiveSystem> m_ArchiveSystem;						// 存档系统，用于管理游戏存档。
		Ref<DialogueSystem> m_DialogueSystem;					// 对话系统，用于处理游戏中的对话和文本显示。
		Ref<LayeredSceneSystem> m_LayeredSceneManager;			// 分层场景管理器，用于管理游戏中的场景和精灵。
		Ref<RenderPipeline> m_RenderPipeline;					// 渲染管线，用于处理游戏的渲染流程。
		Ref<StoryScriptSystem> m_StoryScriptSystem;				// 剧情脚本系统
		Ref<ResourceSystem> m_ResourceSystem;					// 资源系统
		Ref<GalGameUISystem> m_GalGameUISystem;					// 界面系统
	};


}
