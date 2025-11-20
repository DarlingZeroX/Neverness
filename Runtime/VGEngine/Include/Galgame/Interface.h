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
#include "../Interface/EngineInterface.h"
#include "../Interface/RenderInterface.h"
#include "../Interface/GameEngineInterface.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 精灵描述结构体
	 */
	struct SpriteDesc
	{
		String path;      ///< 精灵图片路径
		String layer;     ///< 所在图层
		float alpha = 1.f;    ///< 透明度
		float xoffset = 0.f;  ///< X 偏移
		float yoffset = 0.f;  ///< Y 偏移
		float rotate = 0.f;   ///< 旋转角度
		float zoom = 1.f;     ///< 缩放
		float xzoom = 1.f;    ///< X 方向缩放
		float yzoom = 1.f;    ///< Y 方向缩放
		bool visible = true;  ///< 是否可见
	};

	/**
	 * @brief IGalGameResource 是一个抽象基类，定义了获取 GalGame 资源相关信息的接口。
	 */
	struct IGalGameResource
	{ 
		virtual ~IGalGameResource() = default;

		/**
		 * @brief 获取资源的路径。
		 * @return 资源路径的常量引用字符串。
		 */
		virtual const std::string& GetResourcePath() = 0;

		/**
		 * @brief 获取资源相关的 GameActor 实例。
		 * @return 指向资源相关 GameActor 实例的指针。如果没有可用资源，可能返回空指针。
		 */
		virtual GameActor* GetResourceActor() = 0;

		/**
		 * @brief 获取资源层的名称。
		 * @return 资源层的名称，作为常量引用返回的 std::string。
		 */
		virtual const std::string& GetResourceLayer() = 0;

		/**
		 * @brief 设置资源层的名称。
		 * @param layer 资源层的名称。
		 */
		virtual void SetResourceLayer(const std::string& layer) = 0;
	};

	/**
	 * @brief 定义了一个视觉小说游戏精灵的接口，继承自IGalGameResource。
	 */
	struct IGalGameSprite : public IGalGameResource
	{
		~IGalGameSprite() override = default;
	};

	/**
	 * @brief 定义了一个继承自 IGalGameResource 的音频资源接口。
	 */
	struct IGalGameAudio: public IGalGameResource
	{
		~IGalGameAudio() override = default;
	};

	struct IGalGameVideo: public IGalGameResource
	{
		~IGalGameVideo() override = default;
	};

	struct IGalCharacter
	{
		virtual ~IGalCharacter() = default;
	};

	/**
	 * @brief 定义了一个剧情存档系统接口的结构体。
	 */
	struct IArchiveSystem
	{
		virtual ~IArchiveSystem() = default;
	};

    /**
     * @brief IDialogueSystem 是一个用于管理和控制对话流程的接口，支持角色发言、打字机效果、自动对话、快进等功能。
     */
    struct IDialogueSystem
    {
        virtual ~IDialogueSystem() = default;

		/**
		 * @brief 角色说话
		 * @param character 角色名
		 * @param text 对话内容
		 */
		virtual void CharacterSay(const String& character, const String& text) = 0;
		/**
		 * @brief 开启打字机效果
		 * @param enable 是否启用
		 */
		virtual void EnableTyping(bool enable = true) = 0;
		/**
		 * @brief 完成打字效果
		 */
		virtual void FinishTyping() = 0;
		/**
		 * @brief 是否正在打字
		 * @return 是否正在打字
		 */
		virtual bool IsTypingText() = 0;

		/**
		 * @brief 获取当前对话从开始是第几个对话
		 * @return 当前对话行号
		 */
		virtual uint GetCurrentDialogLine() const = 0;
		/**
		 * @brief 获取对话数量
		 * @return 对话数量
		 */
		virtual uint GetDialogNumber() const = 0;
		/**
		 * @brief 获取对话角色
		 * @param index 对话索引
		 * @return 角色名
		 */
		virtual String GetDialogCharacter(uint index) = 0;
		/**
		 * @brief 获取对话文本
		 * @param index 对话索引
		 * @return 对话内容
		 */
		virtual String GetDialogText(uint index) = 0;
		/**
		 * @brief 获取当前对话角色
		 * @return 当前角色名
		 */
		virtual String GetCurrentCharacter() = 0;
		/**
		 * @brief 获取当前对话文本
		 * @return 当前对话内容
		 */
		virtual String GetCurrentDialogText() = 0;

		/**
		 * @brief 开启自动对话
		 * @param enable 是否启用
		 */
		virtual void AutoDialogue(bool enable) = 0;
		/**
		 * @brief 是否已经开启自动对话
		 * @return 是否自动对话
		 */
		virtual bool IsAutoDialogue() const = 0;

		/**
		 * @brief 开启快进功能
		 * @param enable 是否启用
		 */
		virtual void FastForward(bool enable) = 0;
		/**
		 * @brief 是否开启快进功能
		 * @return 是否快进
		 */
		virtual bool IsFastForward() const = 0;
		/**
		 * @brief 设置快进间隔
		 * @param delay 间隔秒数
		 */
		virtual void SetFastForwardDelay(float delay) = 0;
		/**
		 * @brief 获取快进间隔
		 * @return 间隔秒数
		 */
		virtual float GetFastForwardDelay() const = 0;

		/**
		 * @brief 更新对话系统
		 */
		virtual void Update() = 0;
    };

	struct ISceneSpriteLayer
	{
		virtual ~ISceneSpriteLayer() = default;

		virtual void Clear() = 0;
		virtual bool Add(IGalGameSprite* sprite) = 0;
		virtual bool Remove(IGalGameSprite* sprite) = 0;
	};

	struct ISceneAudioLayer
	{
		virtual ~ISceneAudioLayer() = default;

		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() = 0;
		virtual void Clear() = 0;
		virtual void Add(IGalGameAudio* audio) = 0;
		virtual void StopPlay() = 0;
		virtual bool Remove(IGalGameAudio* audio) = 0;
		virtual bool IsPlayFinished() = 0;
	};

	struct ISceneVideoLayer
	{
		virtual ~ISceneVideoLayer() = default;

		virtual void SetVolume(float volume) = 0;
		virtual float GetVolume() = 0;
		virtual void Clear() = 0;
		virtual void Add(IGalGameVideo* audio) = 0;
		virtual void StopPlay() = 0;
		virtual bool Remove(IGalGameVideo* audio) = 0;
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
		virtual void TraverseSpriteLayer(const String& layer, const std::function<void(IGalGameSprite* actor)>& callback) = 0;

		/**
		 * @brief 遍历所有精灵，并对每个精灵调用回调函数。
		 * @param callback 对每个IGalGameSprite指针执行的回调函数。
		 */
		virtual void TraverseSprite(const std::function<void(IGalGameSprite* actor)>& callback) = 0;

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
		virtual void AddSprite(IGalGameSprite* sprite) = 0;
		/**
		 * @brief 移除精灵对象。
		 * @param sprite 精灵对象指针
		 * @return 是否移除成功
		 */
		virtual bool RemoveSprite(IGalGameSprite* sprite) = 0;
		
		/// @brief 将精灵移动到指定的图层。
		/// @param sprite 要移动的精灵对象指针。
		/// @param layer 目标图层的名称。
		/// @return 如果移动成功，返回 true；否则返回 false。
		virtual bool MoveSpriteToLayer(IGalGameSprite* sprite, const String& layer) = 0;

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
		virtual void TraverseAudioLayer(const String& layer, const std::function<void(IGalGameAudio* audio)>& callback) = 0;

		/**
		 * @brief 遍历所有音频对象，并对每个音频对象执行回调函数。
		 * @param callback 对每个 IGalGameAudio* 音频对象执行的回调函数。
		 */
		virtual void TraverseAudio(const std::function<void(IGalGameAudio* audio)>& callback) = 0;

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
		virtual void AddAudio(IGalGameAudio* audio) = 0;

		/**
		 * @brief 移除音频对象。
		 * @param audio 音频对象指针
		 * @return 是否移除成功
		 */
		virtual bool RemoveAudio(IGalGameAudio* audio) = 0;

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
		virtual void AddVideo(IGalGameVideo* video) = 0;
		virtual bool RemoveVideo(IGalGameVideo* video) = 0;
		virtual void ClearVideoLayer(const String& layer) = 0;
		virtual void ClearAllVideo() = 0;

		virtual void TraverseVideoLayer(const String& layer, const std::function<void(IGalGameVideo* audio)>& callback) = 0;
		virtual void TraverseVideo(const std::function<void(IGalGameVideo* audio)>& callback) = 0;
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

		virtual void TraverseScene(std::function<void(IGalGameResource* actor)> callback) = 0;

		/**
		 * @brief 清空所有资源。
		 */
		virtual void ClearAll() = 0;

		virtual ISceneSpriteManager* GetSpriteManager() = 0;

		virtual ISceneAudioManager* GetAudioManager() = 0;

		virtual ISceneVideoManager* GetVideoManager() = 0;
	};

}
