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
#include "IGameSystem.h"
#include "IStoryScriptSystem.h"
#include "../Include/SaveArchive.h"
#include "VGCore/Interface/GameEngineInterface.h"
//#include "../Game.h"
//#include "ArchiveSystem.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief GalGame引擎接口
	 */
	class IGalGameEngine : public ISubGameEngine
	{
	public:
		~IGalGameEngine() override = default;

		/// 预加载指定路径的资源。
		virtual bool PreLoadResource(const String& path) = 0;

		// 转场相关接口
		virtual bool TransitionCommand(const String& layer, const String& cmd) = 0;
		virtual bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd) = 0;

		// 资源添加相关接口
		virtual IGalSprite* ShowSprite(const std::string& layer, const std::string& path) = 0;	/// 在指定图层上显示精灵，并返回精灵对象指针。
		virtual IGalSprite* ShowColor(const std::string& layer, const float4& color) = 0;				/// 在指定图层上显示颜色，并返回精灵对象指针。
		virtual IGalAudio* PlayAudio(const std::string& layer, const std::string& path) = 0;	/// 播放指定图层上的音频文件。并返回音频对象指针。
		virtual IGalVideo* PlayVideo(const std::string& layer, const std::string& path) = 0;				/// 播放指定图层上的视频文件。并返回视频对象指针。
		virtual IGalCharacter* CreateCharacter(const String& name) = 0;								/// 创建一个具有指定人物名称的 GalCharacter 实例
		
		/// 加载指定的剧情存档对象
		virtual bool LoadArchive(const SaveArchive& archive) = 0;

		// 资源移除相关接口
		virtual bool RemoveSprite(IGalSprite* sprite) = 0;	/// 移除指定的精灵对象。
		virtual bool RemoveAudio(IGalAudio* audio) = 0;		/// 移除指定的音频对象。
		virtual void HideAllCharacterSprite() = 0;			/// 隐藏所有角色精灵。

		// 核心子系统获取接口
		virtual IArchiveSystem* GetArchiveSystem() = 0;
		virtual IDialogueSystem* GetDialogueSystem() = 0;
		virtual ILayeredSceneManager* GetLayeredSceneManager() = 0;
		virtual IStoryScriptSystem* GetStoryScriptSystem() = 0;
		virtual IGalGameUISystem* GetGalGameUISystem() = 0;

		// 剧情脚本相关接口
		virtual bool LoadStoryScript(const String& path) = 0;
		virtual void LoadStoryScriptOnUpdate(const String& path) = 0;
		virtual void ReloadStoryScript() = 0;

		// 引擎生命周期接口
		virtual void Reset() = 0;
		virtual void Wait(float duration) = 0;

		// 场景图像捕获接口
		virtual void CaptureSceneImage() = 0;

		// 引擎上下文获取接口
		virtual ArchiveDataContainer* GetArchiveDataContainer() const = 0;
	};
}
