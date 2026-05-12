/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
 * See the LICENSE file in the project root for details.
 *
 * CORE ABI STABLE
 * DO NOT MODIFY WITHOUT VERSION BUMP
 * 中文：变更须同步升版本、SaveArchive / Lua 绑定与文档。
 */

#pragma once
#include "IGameObject.h"
#include "../Include/SaveArchive.h"

namespace VisionGal::GalGame
{
	/**
	* @brief 定义了一个剧情存档系统接口的结构体。
	*/
	struct IArchiveSystem
	{
		virtual ~IArchiveSystem() = default;

		virtual SaveArchive SaveArchiveByNumber(const String& number) = 0;
		virtual SaveArchive GetArchiveByNumber(const String& number) = 0;
		virtual bool HasArchiveByNumber(const String& number) = 0;
	};

	/**
	* @brief IDialogueSystem 是一个用于管理和控制对话流程的接口，支持角色发言、打字机效果、自动对话、快进等功能。
	*/
	struct IDialogueSystem
	{
		virtual ~IDialogueSystem() = default;

		virtual void CharacterSay(const String& character, const String& text) = 0;	// 角色说话
		virtual void EnableTyping(bool enable = true) = 0;			// 开启打字机效果
		virtual void FinishTyping() = 0;							// 完成打字效果
		virtual bool IsTypingText() = 0;							// 是否正在打字
		virtual void ContinueDialogue() = 0;								// 继续对话，通常用于脚本中调用
		virtual float GetTypingDelay() = 0;								// 获取打字延迟
		virtual void SetTypingDelay(float delay) = 0;						// 设置打字延迟

		virtual uint GetCurrentDialogLine() const = 0;				// 获取当前对话从开始是第几个对话
		virtual uint GetDialogNumber() const = 0;					// 获取对话数量
		virtual String GetDialogCharacter(uint index) = 0;			// 获取对话角色
		virtual String GetDialogText(uint index) = 0;				// 获取对话文本
		virtual String GetCurrentCharacter() = 0;					// 获取当前对话角色
		virtual String GetCurrentDialogText() = 0;					// 获取当前对话文本

		virtual void AutoDialogue(bool enable) = 0;				// 开启自动对话
		virtual bool IsAutoDialogue() const = 0;					// 是否已经开启自动对话

		virtual void FastForward(bool enable) = 0;					// 开启快进功能
		virtual bool IsFastForward() const = 0;					// 是否开启快进功能
		virtual void SetFastForwardDelay(float delay) = 0;			// 设置快进间隔
		virtual float GetFastForwardDelay() const = 0;				// 获取快进间隔

		virtual bool IsVoicing() = 0;										// 是否正在播放语音

		virtual void AddTypingCallback(sol::function callback) = 0;
		virtual void ClearAllTypingCallbacks() = 0;

		// 跳到对话
		virtual void JumpToDialog(const std::string& text) = 0;

		virtual void Reset() = 0;
		virtual void Clear() = 0;
		virtual void Update() = 0;
		virtual void ClearDialogList() = 0;
	};

	struct ISceneSpriteLayer
	{
		virtual ~ISceneSpriteLayer() = default;

		virtual void Clear() = 0;
		virtual bool Add(IGalSprite* sprite) = 0;
		virtual bool Remove(IGalSprite* sprite) = 0;
	};

	struct ISceneAudioLayer
	{
		virtual ~ISceneAudioLayer() = default;

		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() = 0;
		virtual void Clear() = 0;
		virtual void Add(IGalAudio* audio) = 0;
		virtual void StopPlay() = 0;
		virtual bool Remove(IGalAudio* audio) = 0;
		virtual bool IsPlayFinished() = 0;
	};

	struct ISceneVideoLayer
	{
		virtual ~ISceneVideoLayer() = default;

		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() = 0;
		virtual void Clear() = 0;
		virtual void Add(IGalVideo* audio) = 0;
		virtual void StopPlay() = 0;
		virtual bool Remove(IGalVideo* audio) = 0;
		virtual bool IsPlayFinished() = 0;
	};

	/**
	* @brief 用于遍历精灵层和精灵的接口。
	*/
	struct ISceneSpriteManager
	{
		virtual ~ISceneSpriteManager() = default;

		/**
		* @brief 遍历指定图层中的所有精灵，并对每个精灵执行回调函数。
		* @param layer 要遍历的图层名称。
		* @param callback 对每个精灵执行的回调函数，参数为指向精灵对象的指针。
		*/
		virtual void TraverseSpriteLayer(const String& layer, const std::function<void(IGalSprite* actor)>& callback) = 0;

		/**
		* @brief 遍历所有精灵，并对每个精灵调用回调函数。
		* @param callback 对每个IGalGameSprite指针执行的回调函数。
		*/
		virtual void TraverseSprite(const std::function<void(IGalSprite* actor)>& callback) = 0;

		/**
		* @brief 清空指定图层的所有精灵。
		* @param layer 图层名称
		*/
		virtual void ClearSpriteLayer(const String& layer) = 0;
		/**
		* @brief 清空所有精灵。
		*/
		virtual void ClearAllSprite() = 0;

		/**
		* @brief 在指定图层显示精灵。
		* @param layer 图层名称
		* @param actor 精灵对象指针
		*/
		//virtual void ShowSprite(const String& layer, GameActor* actor) = 0;
		/**
		* @brief 添加精灵对象。
		* @param sprite 精灵对象指针
		*/
		virtual void AddSprite(IGalSprite* sprite) = 0;
		/**
		* @brief 移除精灵对象。
		* @param sprite 精灵对象指针
		* @return 是否移除成功
		*/
		virtual bool RemoveSprite(IGalSprite* sprite) = 0;

		/// @brief 将精灵移动到指定的图层。
		/// @param sprite 要移动的精灵对象指针。
		/// @param layer 目标图层的名称。
		/// @return 如果移动成功，返回 true；否则返回 false。
		virtual bool MoveSpriteToLayer(IGalSprite* sprite, const String& layer) = 0;

		virtual void AddSpriteLayer(const String& layer) = 0;

		virtual ISceneSpriteLayer* GetSpriteLayer(const String& layer) = 0;
	};

	/**
	* @brief 音频层遍历接口，提供遍历音频层和音频对象的方法。
	*/
	struct ISceneAudioManager
	{
		virtual ~ISceneAudioManager() = default;

		/**
		* @brief 遍历指定音频层，并对每个音频对象执行回调函数。
		* @param layer 要遍历的音频层名称。
		* @param callback 对每个音频对象调用的回调函数，参数为指向 IGalGameAudio 的指针。
		*/
		virtual void TraverseAudioLayer(const String& layer, const std::function<void(IGalAudio* audio)>& callback) = 0;

		/**
		* @brief 遍历所有音频对象，并对每个音频对象执行回调函数。
		* @param callback 对每个 IGalGameAudio* 音频对象执行的回调函数。
		*/
		virtual void TraverseAudio(const std::function<void(IGalAudio* audio)>& callback) = 0;

		/**
		* @brief 清空指定音频层的所有音频对象。
		* @param layer 音频层名称
		*/
		virtual void ClearSoundLayer(const String& layer) = 0;

		/**
		* @brief 清空所有音频对象。
		*/
		virtual void ClearAllAudio() = 0;

		/**
		* @brief 添加音频对象。
		* @param audio 音频对象指针
		*/
		virtual void AddAudio(IGalAudio* audio) = 0;

		/**
		* @brief 移除音频对象。
		* @param audio 音频对象指针
		* @return 是否移除成功
		*/
		virtual bool RemoveAudio(IGalAudio* audio) = 0;

		virtual void AddAudioLayer(const String& layer) = 0;

		virtual ISceneAudioLayer* GetAudioLayer(const String& layer) = 0;
	};

	/**
	* @brief 用于遍历分层场景的接口，继承自 ISpriteLayerTraverse 和 IAudioLayerTraverse。
	*/
	//struct ILayeredSceneTraverse //: public ISpriteLayerTraverse, public IAudioLayerTraverse
	//{
	//	virtual ~ILayeredSceneTraverse() = default;
	//
	//	/**
	//	 * @brief 遍历场景中的所有资源，并对每个资源执行回调函数。
	//	 * @param callback 对每个 IGalGameResource* 资源执行的回调函数。
	//	 */
	//	virtual void TraverseScene(std::function<void(IGalGameResource* actor)> callback) = 0;
	//};

	struct ISceneVideoManager
	{
		virtual void AddVideo(IGalVideo* video) = 0;
		virtual bool RemoveVideo(IGalVideo* video) = 0;
		virtual void ClearVideoLayer(const String& layer) = 0;
		virtual void ClearAllVideo() = 0;

		virtual void TraverseVideoLayer(const String& layer, const std::function<void(IGalVideo* audio)>& callback) = 0;
		virtual void TraverseVideo(const std::function<void(IGalVideo* audio)>& callback) = 0;
		virtual void AddVideoLayer(const String& layer) = 0;
		virtual ISceneVideoLayer* GetVideoLayer(const String& layer) = 0;

		virtual ~ISceneVideoManager() = default;
	};

	/**
	* @brief 分层场景管理器接口，继承自 ILayeredSceneTraverse。
	*/
	struct ILayeredSceneManager
	{
		virtual ~ILayeredSceneManager() = default;

		virtual void AddCharacter(IGalCharacter* character) = 0;
		virtual void ClearAll() = 0;
		virtual void ClearAllCharacter() = 0;
		virtual void TraverseScene(std::function<void(IGalGameResource* actor)> callback) = 0;
		virtual void TraverseCharacter(const std::function<void(IGalCharacter* character)>& callback) = 0;
		virtual void OnUpdate() = 0;

		virtual ISceneAudioLayer* GetAudioLayer(const String& layer) = 0;
		virtual ISceneSpriteLayer* GetSpriteLayer(const String& layer) = 0;

		virtual ISceneSpriteManager* GetSpriteManager()  = 0;
		virtual ISceneAudioManager* GetAudioManager()  = 0;
		virtual ISceneVideoManager* GetVideoManager()  = 0;
	};

	struct IGalGameUISystem
	{
		virtual ~IGalGameUISystem() = default;

		// 剧情选择UI
		virtual void ShowChoiceUI(const std::string& id, const std::vector<std::string>& options) = 0;
		virtual std::string GetChoiceOptionByIndex(int index) = 0;
		virtual int GetChoiceOptionSize() const = 0;
		virtual void SelectCurrentChoice(int index) = 0;

		// 全屏文字UI
		virtual void ShowFullScreenTextUI(const std::vector<std::string>& texts) = 0;
		virtual std::string GetFullScreenTextItem(int index) = 0;
		virtual int GetFullScreenTextSize() const = 0;

		// 玩家输入UI
		virtual void ShowInputUI(const std::string& id, const std::string& title, const std::string& button) = 0;
		virtual void InputSubmitted(const std::string& text) = 0;
		virtual std::string GetInputTitle() = 0;
		virtual std::string GetInputButtonText() = 0;
	};
}
