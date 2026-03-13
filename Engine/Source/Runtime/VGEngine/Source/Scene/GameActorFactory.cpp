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

#include "Scene/GameActorFactory.h"
#include "Render/Camera.h"
#include "Scene/Components.h"
#include "Engine/Manager.h"
#include "HCore/Interface/HLocalization.h"
#include "Interface/Loader.h"
#include "Galgame/Components.h"
#include "Engine/EngineResource.h"

namespace VisionGal
{
	GameActorFactory::GameActorFactory()
	{
		m_ActorTypeList.push_back("Camera");
		m_ActorTypeList.push_back("Sprite");
		m_ActorTypeList.push_back("Video Player");
	}

	GameActor* GameActorFactory::CreateActor(IScene* scene, const String& type, IEntity* parent)
	{
		if (scene == nullptr)
		{
			H_LOG_ERROR("GameActorFactory传入场景为空");
			return nullptr;
		}

		auto* actor = scene->CreateActor(parent);
		if (type == "Empty")
		{
			actor->SetLabel(Horizon::GetTranslateText("Empty##Actor"));
			return actor;
		}
		if (type == "Camera")
		{
			actor->SetLabel(Horizon::GetTranslateText("Camera"));

			auto com = actor->AddComponent<CameraComponent>();
			auto* viewport = GetViewportManager()->GetMainViewport();
			auto size = viewport->GetState().ViewportSize;
			com->camera = CreateRef<Letterbox2DCamera>(size.x, size.y);

			viewport->AttachCamera(com->camera.get());
			com->camera->AttachViewport(viewport);
		}
		else if (type == "Sprite")
		{
			actor->SetLabel(Horizon::GetTranslateText("Sprite"));

			auto com = actor->AddComponent<SpriteRendererComponent>();

			{
				//String path = "texture/white.png";
				//String resPath = Core::GetEngineResourcePathVFS() + path;
				String path = EngineResource::GetDefaultSpriteTexturePath();

				auto tex = LoadObject<Texture2D>(path);

				com->sprite = Sprite::Create(tex, tex->Size());
			}
		}
		else if (type == "AudioSource")
		{
			actor->SetLabel(Horizon::GetTranslateText("Audio Source"));

			auto com = actor->AddComponent<AudioSourceComponent>();
		}
		else if (type == "VideoPlayer")
		{
			actor->SetLabel(Horizon::GetTranslateText("Video Player"));

			auto com = actor->AddComponent<VideoPlayerComponent>();
		}
		else if (type == "UIDocument")
		{
			actor->SetLabel(Horizon::GetTranslateText("UI Document"));

			auto com = actor->AddComponent<RmlUIDocumentComponent>();
		}
		else if (type == "GalGameEngine")
		{
			actor->SetLabel(Horizon::GetTranslateText("GalGame Engine"));

			auto com = actor->AddComponent<GalGame::GalGameEngineComponent>();
		}

		return actor;
	}

	std::vector<String>& GameActorFactory::GetActorTypeList()
	{
		return m_ActorTypeList;
	}

	GameActorFactory* GetGameActorFactory()
	{
		static GameActorFactory factory;
		return &factory;
	}
}
