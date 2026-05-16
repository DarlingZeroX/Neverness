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
#include "NNRuntimeCore/Interface/SceneInterface.h"
#include "NNEngineLegacy/Include/Animation/Interface/Animation2DScript.h"
#include "NNEngineLegacy/Include/Scene/Components.h"

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
		virtual IGameActor* GetResourceActor() = 0;

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
	struct IGalSprite : public IGalGameResource
	{
		~IGalSprite() override = default;

		virtual IGalSprite* Show() = 0;
		virtual IGalSprite* With(const std::string& transform) = 0;

		virtual Animation2DScript* Animate(const sol::table& targetValue, float duration, std::string tween, int numIterations = 1, bool alternateDirection = true) = 0;

		virtual IGalSprite* SetPosX(float offset) = 0;
		virtual IGalSprite* SetPosY(float offset) = 0;

		virtual float GetPosX() = 0;
		virtual float GetPosY() = 0;
		virtual IGalSprite* SetPosOffsetX(float offset) = 0;
		virtual IGalSprite* SetPosOffsetY(float offset) = 0;

		virtual IGalSprite* SetScale(float scale) = 0;
		virtual IGalSprite* SetScaleWidth(float scale) = 0;
		virtual IGalSprite* SetScaleHeight(float scale) = 0;

		virtual float GetScaleWidth() = 0;
		virtual float GetScaleHeight() = 0;

		virtual IGalSprite* AlignBottom() = 0;
		virtual IGalSprite* AlignBottomWithTexture(Texture2D* tex) = 0;

		virtual TransformComponent* GetTransformComponent() = 0;

		virtual void Cut(const std::string& cut) = 0;
	};

	/**
	 * @brief 定义了一个继承自 IGalGameResource 的音频资源接口。
	 */
	struct IGalAudio: public IGalGameResource
	{
		~IGalAudio() override = default;

		virtual IGalAudio* SetLoop(bool enable) = 0;				// 循环播放
		virtual IGalAudio* Stop() = 0;								// 停止播放
		virtual bool IsPlayingAudio() = 0;							// 是否正在播放音频
		virtual bool IsLooping() = 0;								// 是否循环播放
		virtual IGalAudio* SetVolume(float v) = 0;					// 设置音量
		virtual float GetVolume() = 0;								// 获取音量
		virtual IGalAudio* With(const std::string& transform) = 0;	// 变换
	};

	struct IGalVideo: public IGalGameResource
	{
		~IGalVideo() override = default;

		virtual IGalVideo* SetLoop(bool enable) = 0;
		virtual IGalVideo* Stop() = 0;
		virtual bool IsPlaying() = 0;
		virtual bool IsLooping() = 0;
		virtual IGalVideo* SetVolume(float v) = 0;
		virtual float GetVolume() = 0;
	};

	struct IGalCharacter
	{
		virtual ~IGalCharacter() = default;

		virtual std::string GetName() = 0;
		virtual void SetName(const std::string& name) = 0;
		virtual void Say(const std::string& text) = 0;
		virtual IGalAudio* Voice(const std::string& path) = 0;
		virtual IGalAudio* GetCurrentVoice() = 0;
		virtual void AddFigure(const String& state, const String& spritePath) = 0;

		virtual IGalSprite* ShowFigure(const String& stateOrPath) = 0;
		virtual void HideFigure() = 0;
		virtual IGalSprite* GetCurrentFigure() = 0;

		virtual void AddShowFigureCallback(sol::function callback) = 0;
		virtual void AddHideFigureCallback(sol::function callback) = 0;
		virtual void ClearShowFigureCallbacks() = 0;
		virtual void ClearHideFigureCallbacks() = 0;
	};



}
