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
#include "Interface.h"
#include "Game.h"
#include "ArchiveSystem.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief GalGame引擎接口
	 */
	class IGalGameEngine : public ISubGameEngine
	{
	public:
		~IGalGameEngine() override = default;

		/**
		 * @brief 预加载指定路径的资源。
		 * @param path 要预加载的资源文件路径。
		 * @return 如果资源预加载成功，则返回 true；否则返回 false。
		 */
		virtual bool PreLoadResource(const String& path) = 0;
		 
		/**
		 * @brief 执行转场操作。
		 * @param transition 指向转场对象的引用，用于定义切换的具体方式。
		 * @return 如果转场成功，返回 true；否则返回 false。
		 */
		virtual bool SceneTransition(const Ref<ISceneTransition>& transition) = 0;

		/**
		 * @brief 执行指定图层上的转场命令。
		 * @param layer 要应用转场命令的图层名称。
		 * @param cmd 要执行的转场命令。
		 * @return 如果命令执行成功，则返回 true；否则返回 false。
		 */
		virtual bool TransitionCommand(const String& layer, const String& cmd) = 0;

		/**
		 * @brief 获取存档系统的指针（纯虚函数）。
		 * @return 指向存档系统（IArchiveSystem）的指针。
		 */
		virtual IArchiveSystem* GetArchiveSystem() = 0;

		/**
		 * @brief 获取对话系统的指针（纯虚函数）。
		 * @return 指向IDialogueSystem对象的指针。
		 */
		virtual IDialogueSystem* GetDialogueSystem() = 0;

		/**
		 * @brief 获取分层场景管理器的指针（纯虚函数）。
		 * @return 指向分层场景管理器（ILayeredSceneManager）的指针。
		 */
		virtual ILayeredSceneManager* GetLayeredSceneManager() = 0;

		/**
		 * @brief 在指定图层上显示精灵，并返回精灵对象指针。
		 * @param layer 要显示精灵的图层名称。
		 * @param path 精灵图片的文件路径。
		 * @return 指向显示的精灵对象的指针。
		 */
		virtual GalSprite* ShowSprite(const std::string& layer, const std::string& path) = 0;

		/**
		 * @brief 播放指定图层上的音频文件。
		 * @param layer 要播放音频的图层名称。
		 * @param path 音频文件的路径。
		 * @return 返回指向播放音频的 GalAudio 对象的指针。
		 */
		virtual GalAudio* PlayAudio(const std::string& layer, const std::string& path) = 0;

		/**
		 * @brief 创建一个具有指定名称的 GalCharacter 实例（纯虚函数）。
		 * @param name 角色的名称。
		 * @return 指向新创建的 GalCharacter 对象的指针。
		 */
		virtual GalCharacter* CreateCharacter(const String& name) = 0;

		/**
		 * @brief 隐藏所有角色精灵。
		 */
		virtual void HideAllCharacterSprite() = 0;

		/**
		 * @brief 移除指定的精灵对象。
		 * @param sprite 要移除的 GalSprite 指针。
		 * @return 如果成功移除精灵，则返回 true；否则返回 false。
		 */
		virtual bool RemoveSprite(GalSprite* sprite) = 0;

		/**
		 * @brief 移除指定的音频对象。
		 * @param audio 要移除的 GalAudio 指针。
		 * @return 如果成功移除音频，则返回 true；否则返回 false。
		 */
		virtual bool RemoveAudio(GalAudio* audio) = 0;

		/**
		 * @brief 重新加载剧情脚本的纯虚函数。
		 */
		virtual void ReloadStoryScript() = 0;

		/**
		 * @brief 加载指定路径的剧情脚本。
		 * @param path 要加载的剧情脚本文件的路径。
		 * @return 如果脚本加载成功，返回 true；否则返回 false。
		 */
		virtual bool LoadStoryScript(const String& path) = 0;

		/**
		 * @brief 在更新时加载指定路径的剧情脚本（纯虚函数）。
		 * @param path 要加载的剧情脚本文件的路径。
		 */
		virtual void LoadStoryScriptOnUpdate(const String& path) = 0;

		/**
		 * @brief 加载指定的剧情存档对象。
		 * @param archive 要加载的 SaveArchive 类型的剧情存档对象。
		 * @return 如果剧情存档加载成功，则返回 true；否则返回 false。
		 */
		virtual bool LoadArchive(const SaveArchive& archive) = 0;

		/**
		 * @brief 重置引擎的状态，通常用于将引擎恢复到初始状态。
		 */
		virtual void Reset() = 0;

		/**
		 * @brief 等待指定的时间长度。
		 * @param duration 等待的时间长度（以秒为单位）。
		 */
		virtual void Wait(float duration) = 0;
	};
}
