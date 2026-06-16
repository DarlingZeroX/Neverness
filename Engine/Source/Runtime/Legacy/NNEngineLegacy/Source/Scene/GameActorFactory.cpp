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
#include "NNCore/Interface/HLocalization.h"
#include "NNRuntimeCore/Interface/Loader.h"
//#include "Galgame/Components.h"
#include "Engine/EngineResource.h"

namespace NN::Runtime
{
	GameActorFactory::GameActorFactory()
	{
		m_ActorTypeList.push_back("Camera");
		m_ActorTypeList.push_back("Sprite");
		m_ActorTypeList.push_back("Video Player");
	}

	IGameActor* GameActorFactory::CreateActor(IScene* scene, const String& type, IEntity* parent)
	{
		if (scene == nullptr)
		{
			H_LOG_ERROR("GameActorFactory传入场景为空");
			return nullptr;
		}

		auto* actor = scene->CreateActor(parent);
		if (type == "Empty")
		{
			actor->SetLabel(NN::Core::GetTranslateText("Empty##Actor"));
			return actor;
		}
		if (type == "Camera")
		{
			actor->SetLabel(NN::Core::GetTranslateText("Camera"));

			auto com = actor->AddComponent<CameraComponent>();
			auto* viewport = GetViewportManager()->GetMainViewport();
			auto size = viewport->GetState().ViewportSize;
			com->camera = MakeRef<Letterbox2DCamera>(size.x, size.y);

			viewport->AttachCamera(com->camera.get());
			com->camera->AttachViewport(viewport);
		}
		else if (type == "Sprite")
		{
			actor->SetLabel(NN::Core::GetTranslateText("Sprite"));

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
			actor->SetLabel(NN::Core::GetTranslateText("Audio Source"));

			auto com = actor->AddComponent<AudioSourceComponent>();
		}
		else if (type == "VideoPlayer")
		{
			actor->SetLabel(NN::Core::GetTranslateText("Video Player"));

			auto com = actor->AddComponent<VideoPlayerComponent>();
		}
		else if (type == "UIDocument")
		{
			actor->SetLabel(NN::Core::GetTranslateText("UI Document"));

			auto com = actor->AddComponent<RmlUIDocumentComponent>();
		}
		//else if (type == "GalGameEngine")
		//{
		//	actor->SetLabel(NN::Core::GetTranslateText("GalGame Engine"));
		//
		//	auto com = actor->AddComponent<GalGame::GalGameEngineComponent>();
		//}

		for (const auto& creator : m_ActorCreators)
		{
			if (creator->GetType() == type)
			{
				creator->BuildActor(actor);
			}
		}

		return actor;
	}

	void GameActorFactory::AddGameActorCreator(const Ref<IGameActorBuilder>& creator)
	{
		m_ActorCreators.push_back(creator);
		m_ActorTypeList.push_back(creator->GetType());
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
