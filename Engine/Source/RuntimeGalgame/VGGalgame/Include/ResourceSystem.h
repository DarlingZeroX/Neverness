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
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "SceneSystem/LayeredSceneSystem.h"
#include "../VGGalgameConfig.h"
#include "VGEngine/Include/Scene/Scene.h"

namespace VisionGal::GalGame
{
	struct IGalGameEngine;

	class VG_GALGAME_API ResourceSystem 
	{
	public:
		ResourceSystem();
		~ResourceSystem() = default;
		ResourceSystem(const ResourceSystem&) = delete;
		ResourceSystem& operator=(const ResourceSystem&) = delete;
		ResourceSystem(ResourceSystem&&) noexcept = default;
		ResourceSystem& operator=(ResourceSystem&&) noexcept = default;

		/// 中文：hostEngine 用于构造 GalSprite/GalAudio/GalVideo（需 IGalGameEngine*）；Phase 8 起不再从 GalGameContext 取 Engine。
		void Initialize(const Ref<GalGameContext>& galCtx, const Ref<LayeredSceneSystem>& sceneSystem, IGalGameEngine* hostEngine);

		/// 预加载指定路径的资源。
		bool PreLoadResource(const String& path);
		// 资源添加相关接口
		GalSprite* ShowSprite(const std::string& layer, const std::string& path);	/// 在指定图层上显示精灵，并返回精灵对象指针。
		GalSprite* ShowColor(const std::string& layer, const float4& color);				/// 在指定图层上显示颜色，并返回精灵对象指针。
		GalAudio* PlayAudio(const std::string& layer, const std::string& path);	/// 播放指定图层上的音频文件。并返回音频对象指针。
		GalVideo* PlayVideo(const std::string& layer, const std::string& path);				/// 播放指定图层上的视频文件。并返回视频对象指针。

		// 资源移除相关接口
		bool RemoveSprite(GalSprite* sprite);	/// 移除指定的精灵对象。
		bool RemoveAudio(GalAudio* audio);		/// 移除指定的音频对象。
	private:
		IGameActor* CreateSpriteImp(const std::string& path);
		GalSprite* AddSprite(IGameActor* sprite, const std::string& layer, const std::string& path);
		IGameActor* CreateAudioImp(const std::string& path);
		GalAudio* AddAudio(IGameActor* audio, const std::string& layer, const std::string& path);
		IGameActor* CreateVideoImp(const std::string& path);
		GalVideo* AddVideo(IGameActor* audio, const std::string& layer, const std::string& path);
	private:
		Scene* m_Scene;

		Ref<GalGameContext> m_GalGameContext;
		IGalGameEngine* m_HostEngine = nullptr;

		Ref<LayeredSceneSystem> m_LayeredSceneManager;
	};


}
