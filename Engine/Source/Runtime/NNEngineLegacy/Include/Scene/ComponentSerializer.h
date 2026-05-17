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
#include "NNRuntimeAsset/Interface/ISceneSerializer.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include "Components.h"
#include <NNCore/Interface/HSerialization.h>

namespace NN::Runtime
{
	struct TransformComponentSerializer : public IEntityComponentSerializer<TransformComponent>
	{
		TransformComponentSerializer() = default;
		TransformComponentSerializer(const TransformComponentSerializer&) = default;
		TransformComponentSerializer& operator=(const TransformComponentSerializer&) = default;
		TransformComponentSerializer(TransformComponentSerializer&&) noexcept = default;
		TransformComponentSerializer& operator=(TransformComponentSerializer&&) noexcept = default;
		~TransformComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<TransformComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};
	
	struct CameraComponentSerializer: public IEntityComponentSerializer<CameraComponent>
	{
		CameraComponentSerializer() = default;
		CameraComponentSerializer(const CameraComponentSerializer&) = default;
		CameraComponentSerializer& operator=(const CameraComponentSerializer&) = default;
		CameraComponentSerializer(CameraComponentSerializer&&) noexcept = default;
		CameraComponentSerializer& operator=(CameraComponentSerializer&&) noexcept = default;
		~CameraComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<CameraComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	struct ScriptComponentSerializer : public IEntityComponentSerializer<ScriptComponent>
	{
		ScriptComponentSerializer() = default;
		ScriptComponentSerializer(const ScriptComponentSerializer&) = default;
		ScriptComponentSerializer& operator=(const ScriptComponentSerializer&) = default;
		ScriptComponentSerializer(ScriptComponentSerializer&&) noexcept = default;
		ScriptComponentSerializer& operator=(ScriptComponentSerializer&&) noexcept = default;
		~ScriptComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<ScriptComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};
	
	struct SpriteRendererComponentSerializer: public IEntityComponentSerializer<SpriteRendererComponent>
	{
		SpriteRendererComponentSerializer() = default;
		SpriteRendererComponentSerializer(const SpriteRendererComponentSerializer&) = default;
		SpriteRendererComponentSerializer& operator=(const SpriteRendererComponentSerializer&) = default;
		SpriteRendererComponentSerializer(SpriteRendererComponentSerializer&&) noexcept = default;
		SpriteRendererComponentSerializer& operator=(SpriteRendererComponentSerializer&&) noexcept = default;
		~SpriteRendererComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<SpriteRendererComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	struct AudioSourceComponentSerializer : public IEntityComponentSerializer<AudioSourceComponent>
	{
		AudioSourceComponentSerializer() = default;
		AudioSourceComponentSerializer(const AudioSourceComponentSerializer&) = default;
		AudioSourceComponentSerializer& operator=(const AudioSourceComponentSerializer&) = default;
		AudioSourceComponentSerializer(AudioSourceComponentSerializer&&) noexcept = default;
		AudioSourceComponentSerializer& operator=(AudioSourceComponentSerializer&&) noexcept = default;
		~AudioSourceComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<AudioSourceComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	struct VideoPlayerComponentSerializer : public IEntityComponentSerializer<VideoPlayerComponent>
	{
		VideoPlayerComponentSerializer() = default;
		VideoPlayerComponentSerializer(const VideoPlayerComponentSerializer&) = default;
		VideoPlayerComponentSerializer& operator=(const VideoPlayerComponentSerializer&) = default;
		VideoPlayerComponentSerializer(VideoPlayerComponentSerializer&&) noexcept = default;
		VideoPlayerComponentSerializer& operator=(VideoPlayerComponentSerializer&&) noexcept = default;
		~VideoPlayerComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<VideoPlayerComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	struct RmlUIDocumentComponentSerializer : public IEntityComponentSerializer<RmlUIDocumentComponent>
	{
		RmlUIDocumentComponentSerializer() = default;
		RmlUIDocumentComponentSerializer(const RmlUIDocumentComponentSerializer&) = default;
		RmlUIDocumentComponentSerializer& operator=(const RmlUIDocumentComponentSerializer&) = default;
		RmlUIDocumentComponentSerializer(RmlUIDocumentComponentSerializer&&) noexcept = default;
		RmlUIDocumentComponentSerializer& operator=(RmlUIDocumentComponentSerializer&&) noexcept = default;
		~RmlUIDocumentComponentSerializer() override = default;

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<RmlUIDocumentComponentSerializer>();
		};
		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	//class SceneSerializer : public ISceneSerializer
	//{
	//public:
	//	SceneSerializer();
	//	SceneSerializer(const SceneSerializer&) = default;
	//	SceneSerializer& operator=(const SceneSerializer&) = default;
	//	SceneSerializer(SceneSerializer&&) noexcept = default;
	//	SceneSerializer& operator=(SceneSerializer&&) noexcept = default;
	//	~SceneSerializer() override = default;
	//
	//	int GetSerializerNumber() const override;
	//
	//	int SerializeScene(cereal::JSONOutputArchive& archive, IScene* scene) override;
	//	int DeserializeScene(cereal::JSONInputArchive& archive, IScene* scene) override;
	//private:
	//	std::unordered_map<String, Ref<ISceneSegmentSerializer>> m_SegmentSerializers;
	//};
}
