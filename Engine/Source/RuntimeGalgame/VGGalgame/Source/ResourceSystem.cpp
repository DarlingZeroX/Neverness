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

#include "ResourceSystem.h"
#include "VGGalgameCore/Include/GalGameEngineAccess.h"
#include "VGCore/Interface/Loader.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGEngine/Include/Engine/EngineResource.h"
#include "VGEngine/Include/Engine/AudioPlayer.h"
#include "VGEngine/Include/Engine/VideoPlayer.h"

namespace VisionGal::GalGame
{
	ResourceSystem::ResourceSystem()
	{
	}

	void ResourceSystem::Initialize(const Ref<GalGameContext>& galCtx, const Ref<LayeredSceneSystem>& sceneSystem, IGalGameEngine* hostEngine)
	{
		m_GalGameContext = galCtx;
		m_LayeredSceneManager = sceneSystem;
		m_HostEngine = hostEngine;

		EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
			{
				switch (evt.EventType)
				{
				case EngineEventType::MainSceneChanged:
					m_Scene = dynamic_cast<Scene*>(evt.Scene);
					break;
				}
			});
	}

	bool ResourceSystem::PreLoadResource(const String& path)
	{
		if (Horizon::HStringTools::EndWith(path, ".png") || Horizon::HStringTools::EndWith(path, ".jpg") || Horizon::HStringTools::EndWith(path, ".bmp") || Horizon::HStringTools::EndWith(path, ".tga"))
		{
			std::thread thread([path, this]()
				{
					String resPath = Core::GetAssetsPathVFS() + path;
					AssetManager::GetInstance()->LoadAsset<TextureAsset>(resPath);
				});

			thread.detach();
		}

		return true;
	}

	IGameActor* ResourceSystem::CreateSpriteImp(const std::string& path)
	{
		// 读取纹理资产
		auto tex = LoadObject<Texture2D>(path);
		if (tex == nullptr)
		{
			H_LOG_WARN("加载图片失败: %s", path.c_str());
			return nullptr;
		}

		// 创建精灵角色
		auto* actor = m_Scene->CreateActor();
		actor->SetLabel(path);

		// 添加必要组件
		actor->AddComponent<AnimationScriptComponent>();
		actor->AddComponent<SpriteRendererComponent>()->sprite = Sprite::Create(tex, tex->Size());

		return actor;
	}

	GalSprite* ResourceSystem::AddSprite(IGameActor* actor, const std::string& layer, const std::string& path)
	{
		if (actor == nullptr)
			return nullptr;

		// 创建GalGame的图片类
		GalSprite* sprite = new GalSprite(m_HostEngine, layer, path);
		sprite->m_Actor = actor;
		sprite->m_GalState = &m_GalGameContext->runtimeState;

		// 设置渲染管线
		auto* spriteCom = actor->GetComponent<SpriteRendererComponent>();
		if (spriteCom) {
			spriteCom->pipelineIndex = static_cast<uint>(RenderPipelineIndex::GalGamePipeline);

			// 设置屏幕专属渲染管线
			if (layer == "Screen")
			{
				spriteCom->pipelineIndex = static_cast<uint>(RenderPipelineIndex::ScreenPipeline);
			}
		}

		// 对齐底部
		sprite->AlignBottom();

		// 添加到管理器
		m_LayeredSceneManager->GetSpriteManager()->AddSprite(sprite);

		return sprite;
	}

	IGameActor* ResourceSystem::CreateAudioImp(const std::string& resPath)
	{
		// 读取音频资产
		auto audioClip = LoadObject<VGAudioClip>(resPath);
		if (audioClip == nullptr)
		{
			H_LOG_WARN("加载音频失败: %s", resPath.c_str());
			return nullptr;
		}

		// 创建音频角色
		auto* actor = m_Scene->CreateActor();
		actor->SetLabel(resPath);

		// 添加音频源组件
		auto* audioSource = actor->AddComponent<AudioSourceComponent>();
		audioSource->audioClip = audioClip;

		return actor;
	}

	GalAudio* ResourceSystem::AddAudio(IGameActor* actor, const std::string& layer, const std::string& path)
	{
		// 创建GalGame的音频类
		GalAudio* audio = new GalAudio(m_HostEngine, layer, path);
		audio->m_Actor = actor;

		// 添加到管理器
		m_LayeredSceneManager->GetAudioManager()->AddAudio(audio);

		return audio;
	}

	IGameActor* ResourceSystem::CreateVideoImp(const std::string& resPath)
	{
		// 读取视频资产
		auto videoClip = LoadObject<FVideoClip>(resPath);
		if (videoClip == nullptr)
		{
			H_LOG_WARN("加载视频失败: %s", resPath.c_str());
			return nullptr;
		}

		// 创建音频角色
		auto* actor = m_Scene->CreateActor();
		actor->SetLabel(resPath);

		// 添加视频源组件
		auto* audioSource = actor->AddComponent<VideoPlayerComponent>();
		audioSource->videoClip = videoClip;

		return actor;
	}

	GalVideo* ResourceSystem::AddVideo(IGameActor* actor, const std::string& layer, const std::string& path)
	{
		// 创建GalGame的视频类
		GalVideo* video = new GalVideo(m_HostEngine, layer, path);
		video->m_Actor = actor;

		// 添加到管理器
		m_LayeredSceneManager->GetVideoManager()->AddVideo(video);

		return video;
	}

	GalSprite* ResourceSystem::ShowSprite(const std::string& layer, const std::string& path)
	{
		//String resPath = Core::GetAssetsPathVFS() + path;
		String resPath = path;
		IGameActor* actor = CreateSpriteImp(resPath);

		if (actor == nullptr)
			return nullptr;

		return AddSprite(actor, layer, resPath);
	}

	GalSprite* ResourceSystem::ShowColor(const std::string& layer, const float4& color)
	{
		String resPath = EngineResource::GetDefaultSpriteTexturePath();
		IGameActor* actor = CreateSpriteImp(resPath);

		if (actor == nullptr)
			return nullptr;

		GalSprite* sprite = AddSprite(actor, layer, resPath);

		// 设置全屏颜色
		actor->GetComponent<TransformComponent>()->scale = float3(999999, 999999, 1);
		actor->GetComponent<SpriteRendererComponent>()->color = color;

		return sprite;
	}

	GalAudio* ResourceSystem::PlayAudio(const std::string& layer, const std::string& path)
	{
		String resPath = path;
		IGameActor* actor = CreateAudioImp(resPath);

		if (actor == nullptr)
			return nullptr;

		// 播放
		actor->GetComponent<AudioSourceComponent>()->Play();

		return AddAudio(actor, layer, resPath);
	}

	GalVideo* ResourceSystem::PlayVideo(const std::string& layer, const std::string& path)
	{
		String resPath = path;
		IGameActor* actor = CreateVideoImp(resPath);

		if (actor == nullptr)
			return nullptr;

		// 播放
		actor->GetComponent<VideoPlayerComponent>()->Play();

		return AddVideo(actor, layer, resPath);
	}

	bool ResourceSystem::RemoveSprite(GalSprite* sprite)
	{
		return m_LayeredSceneManager->GetSpriteManager()->RemoveSprite(sprite);
	}

	bool ResourceSystem::RemoveAudio(GalAudio* audio)
	{
		return m_LayeredSceneManager->GetAudioManager()->RemoveAudio(audio);
	}

}
