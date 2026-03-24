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

#include "Scene/ComponentSerializer.h"

#include "Engine/Manager.h"
#include "UI/UISystem.h"
#include "VGCore/Interface/Loader.h"
#include "Lua/LuaScript.h"
#include "Render/Camera.h"

#include "Galgame/Components.h"
#include "Engine/AudioPlayer.h"
#include "Engine/VideoPlayer.h"

namespace VisionGal
{
	void TransformComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		TransformComponent& com = world->emplace<TransformComponent>(actor->GetEntity());
		com = m_ComponentMap[id];
		com.is_dirty = true;

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void CameraComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		CameraComponent& com = world->emplace<CameraComponent>(actor->GetEntity());
		CameraComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;

		if (deserializeComponent.__DeserializeData.HasCamera)
		{
			if (deserializeComponent.__DeserializeData.m_CameraType == "Letterbox2D")
			{
				auto* viewport = GetViewportManager()->GetMainViewport();
				auto size = viewport->GetState().ViewportSize;
				com.camera = MakeRef<Letterbox2DCamera>(size.x, size.y);

				viewport->AttachCamera(com.camera.get());
				com.camera->AttachViewport(viewport);
			}
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void ScriptComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		ScriptComponent& com = world->emplace<ScriptComponent>(actor->GetEntity());
		ScriptComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;

		for (auto& data: deserializeComponent.__DeserializeData.m_ScriptsData)
		{
			if (data.Type == "LuaScript")
			{
				if (actor->GetComponent<ScriptComponent>() == nullptr)
				{
					actor->AddComponent<ScriptComponent>();
				}

				auto* com = actor->GetComponent<ScriptComponent>();
				auto script = LuaScript::LoadFromFile(data.Path);

				if (script)
				{
					com->scripts.push_back(script);
					script->SetVariable(data.Variables);
				}
			}
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void SpriteRendererComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		SpriteRendererComponent& com = world->emplace<SpriteRendererComponent>(actor->GetEntity());
		SpriteRendererComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;
		if (deserializeComponent.__DeserializeData.HasSprite)
		{
			auto tex = LoadObject<Texture2D>(deserializeComponent.__DeserializeData.m_SpriteTexturePath);
			com.sprite = MakeRef<Sprite>(tex, deserializeComponent.__DeserializeData.m_SpriteSize, deserializeComponent.__DeserializeData.m_SpritePosition);

			if (tex == nullptr)
			{
				auto tex = LoadObject<Texture2D>(Core::GetDefaultSpriteTexturePath());

				if (tex == nullptr)
				{
					H_LOG_ERROR("Failed to load default sprite texture.");
					throw "Failed to load default sprite texture.";
				}

				com.sprite = Sprite::Create(tex, tex->Size());
			}
		}
		else
		{
			auto tex = LoadObject<Texture2D>(Core::GetDefaultSpriteTexturePath());
			com.sprite = Sprite::Create(tex, tex->Size());
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void AudioSourceComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		AudioSourceComponent& com = world->emplace<AudioSourceComponent>(actor->GetEntity());
		AudioSourceComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;
		if (deserializeComponent.__DeserializeData.HasAudio)
		{
			auto tex = LoadObject<VGAudioClip>(deserializeComponent.__DeserializeData.m_AudioPath);
			com.audioClip = tex;
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void VideoPlayerComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		VideoPlayerComponent& com = world->emplace<VideoPlayerComponent>(actor->GetEntity());
		VideoPlayerComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;
		if (deserializeComponent.__DeserializeData.HasVideo)
		{
			auto tex = LoadObject<FVideoClip>(deserializeComponent.__DeserializeData.m_VideoPath);
			com.videoClip = tex;
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	void RmlUIDocumentComponentSerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
		auto* world = scene->GetWorld();
		RmlUIDocumentComponent& com = world->emplace<RmlUIDocumentComponent>(actor->GetEntity());
		RmlUIDocumentComponent& deserializeComponent = m_ComponentMap[id];

		com = deserializeComponent;
		if (deserializeComponent.__DeserializeData.HasDocument)
		{
			com.document = MakeRef<RmlUIDocument>();
			com.document->SetResourcePath(deserializeComponent.__DeserializeData.m_DocumentPath);
			//UISystem::Get()->LoadUIDocument(deserializeComponent.__DeserializeData.m_DocumentPath);
			//UISystem::Get()->ShowUIDocument(com.document);
		}

		scene->UpdateDeserializeComponent(actor, &com);
	}

	//SceneSerializer::SceneSerializer()
	//{
	//	m_SegmentSerializers[EntitySerializer{}.GetSegmentType()] = MakeRef<EntitySerializer>();
	//	m_SegmentSerializers[TransformComponentSerializer{}.GetSegmentType()] = MakeRef<TransformComponentSerializer>();
	//	m_SegmentSerializers[CameraComponentSerializer{}.GetSegmentType()] = MakeRef<CameraComponentSerializer>();
	//	m_SegmentSerializers[SpriteRendererComponentSerializer{}.GetSegmentType()] = MakeRef<SpriteRendererComponentSerializer>();
	//	m_SegmentSerializers[ScriptComponentSerializer{}.GetSegmentType()] = MakeRef<ScriptComponentSerializer>();
	//	m_SegmentSerializers[AudioSourceComponentSerializer{}.GetSegmentType()] = MakeRef<AudioSourceComponentSerializer>();
	//	m_SegmentSerializers[VideoPlayerComponentSerializer{}.GetSegmentType()] = MakeRef<VideoPlayerComponentSerializer>();
	//	m_SegmentSerializers[RmlUIDocumentComponentSerializer{}.GetSegmentType()] = MakeRef<RmlUIDocumentComponentSerializer>();
	//
	//	m_SegmentSerializers[GalGame::GalGameEngineComponentSerializer{}.GetSegmentType()] = MakeRef<GalGame::GalGameEngineComponentSerializer>();
	//}
	//
	//int SceneSerializer::GetSerializerNumber() const
	//{
	//	return m_SegmentSerializers.size();
	//}
	//
	//int SceneSerializer::SerializeScene(cereal::JSONOutputArchive& archive, IScene* scene)
	//{
	//	archive(cereal::make_nvp("SegmentNumber", GetSerializerNumber()));
	//
	//	for (auto& part: m_SegmentSerializers)
	//	{
	//		auto* serializer = part.second.get();
	//
	//		archive(cereal::make_nvp("SegmentType", serializer->GetSegmentType()));
	//		serializer->WriteSegment(archive, scene);
	//	}
	//
	//	return 0;
	//}
	//
	//int SceneSerializer::DeserializeScene(cereal::JSONInputArchive& archive, IScene* scene)
	//{
	//	int segmentNumber = 0;
	//	archive(segmentNumber);
	//
	//	SceneDeserializeDataContainer container;
	//	container.scene = scene;
	//
	//	// 反序列化到场景数据容器
	//	for (int i = 0; i < segmentNumber; i++)
	//	{
	//		std::string segmentType;
	//		archive(segmentType);
	//
	//		m_SegmentSerializers[segmentType]->ReadSegment(archive, container);
	//	} 
	//
	//	// 创建反序列化后的实体
	//	for (auto& entity: container.entities)
	//	{
	//		IGameActor* actor = scene->CreateDeserializeActor(entity);
	//		auto* entityData = actor->GetComponent<Horizon::HEntityObjectData>();
	//
	//		for (auto& comID: entity.ComponentIDs)
	//		{
	//			auto result = container.componentMap.find(comID);
	//			if (result != container.componentMap.end())
	//			{
	//				//auto* serializer = container.componentMap[comID];
	//				auto* serializer = result->second;
	//				entityData->ComponentIDs.push_back(comID);
	//				entityData->ComponentTypes.push_back(serializer->GetSegmentType());
	//
	//				serializer->AddActorSerializeComponent(scene, actor, comID);
	//			}
	//		}
	//	}
	//
	//	scene->UpdateDeserializeActorRelationship();
	//
	//	return 0;
	//}
}
