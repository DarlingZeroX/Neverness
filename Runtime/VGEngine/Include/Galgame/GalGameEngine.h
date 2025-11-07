#pragma once
#include "GalGameEngineInterface.h"
#include "../EngineConfig.h"
#include "../Scene/Scene.h"
#include "../Render/RenderCore.h"
#include "../Utils/TransitionHelper.h"
#include "RenderPipeline.h"
#include "LayeredSceneManager.h"
#include "ArchiveSystem.h"
#include "DialogueSystem.h"
#include "StoryScript.h"

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
		 
		bool PreLoadResource(const String& path) override;	/// 预加载指定路径的资源。

		bool SceneTransition(const Ref<ISceneTransition>& transition) override;		/// 用自定义转场对象执行转场操作。
		bool TransitionCommand(const String& layer, const String& cmd) override;	/// 执行指定图层上的转场命令。
		bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd);

		GalSprite* ShowSprite(const std::string& layer, const std::string& path) override;	/// 在指定图层上显示精灵，并返回精灵对象指针。
		GalSprite* ShowColor(const std::string& layer, const float4& color);
		GalAudio* PlayAudio(const std::string& layer, const std::string& path) override;	/// 播放指定图层上的音频文件。并返回音频对象指针。

		GalCharacter* CreateCharacter(const String& name) override;	/// 创建一个具有指定人物名称的 GalCharacter 实例
		void HideAllCharacterSprite() override;	/// 隐藏所有角色精灵。

		bool RemoveSprite(GalSprite* sprite) override;	/// 移除指定的精灵对象。
		bool RemoveAudio(GalAudio* audio) override;	/// 移除指定的音频对象。

		IArchiveSystem* GetArchiveSystem() override;	/// 获取存档系统的指针
		IDialogueSystem* GetDialogueSystem() override;	/// 获取对话系统的指针
		ILayeredSceneManager* GetLayeredSceneManager() override;

		void Initialize(IGameEngineContext* context);

		void OnRender() override;
		void OnUpdate(float deltaTime) override;

		void ReloadStoryScript() override;	/// 重新加载剧情脚本
		bool LoadStoryScript(const String& path) override;	/// 加载指定路径的剧情脚本
		void LoadStoryScriptOnUpdate(const String& path) override;	/// 在更新时加载指定路径的剧情脚本
		bool LoadArchive(const SaveArchive& archive) override;	/// 加载指定的剧情存档对象

		void Reset() override;	/// 重置引擎的状态，通常用于将引擎恢复到初始状态。
		void Wait(float duration) override;	/// 等待指定的时间长度。

		void CaptureSceneImage();
	private:
		GameActor* CreateSpriteInline(const std::string& path);
		GalSprite* AddSprite(GameActor* sprite,const std::string& layer, const std::string& path);

		void OnMainSceneChanged(const EngineEvent& evt);
		void CreateSubsystem(IGameEngineContext* context, Rml::Context* uiContext);

		void OneRenderSceneCallback(OpenGL::RenderTarget2D* rt);
		void AddUpdateCallback(const std::function<void()>& callback);

	private:
		struct WaitStruct
		{
			bool IsWait = false;
			TransitionHelper Helper;
		};

		IGameEngineContext* m_EngineContext;					// 引擎上下文，包含了引擎的各种系统和状态。
		Ref<VGFX::TexturePixels> m_CapturedSceneImage;			// 捕获的场景图像数据，用于存储当前场景的截图。

		// 引擎核心子系统
		Ref<ArchiveSystem> m_ArchiveSystem;						// 存档系统，用于管理游戏存档。
		Ref<DialogueSystem> m_DialogueSystem;					// 对话系统，用于处理游戏中的对话和文本显示。
		Ref<LayeredSceneManager> m_LayeredSceneManager;			// 分层场景管理器，用于管理游戏中的场景和精灵。
		Ref<RenderPipeline> m_RenderPipeline;					// 渲染管线，用于处理游戏的渲染流程。

		Scene* m_Scene;											// 当前的场景对象，表示游戏中的一个具体场景。
		Ref<LuaStoryScript> m_StoryScript;						// 当前加载的剧情脚本对象，使用 Lua 脚本语言编写。

		std::vector<Ref<GalCharacter>> m_Characters;			// 当前游戏中的所有角色列表，存储了所有创建的 GalCharacter 实例。

		std::vector<std::function<void()>> m_UpdateCallback;	// 更新回调函数列表，用于在每帧更新时执行特定的操作。

		WaitStruct m_Wait;										// 等待结构体，用于处理等待状态和相关的时间管理。
		bool m_IsEngineEnable = false;							// 引擎是否启用的标志，指示引擎是否处于活动状态。
	};


}
