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
#include "ISceneSerializer.h"
#include "VGCore/Include/Core/Core.h"
#include "../../Scene/Components.h"
#include <HCore/Interface/HSerialization.h>

namespace VisionGal
{
	struct EntitySerializer : public ISceneSegmentSerializer
	{
		EntitySerializer() = default;
		EntitySerializer(const EntitySerializer&) = default;
		EntitySerializer& operator=(const EntitySerializer&) = default;
		EntitySerializer(EntitySerializer&&) noexcept = default;
		EntitySerializer& operator=(EntitySerializer&&) noexcept = default;
		~EntitySerializer() override = default;

		String GetSegmentType() override;

		int WriteSegment(cereal::JSONOutputArchive& archive, Scene* scene) override;
		int WriteSegment(cereal::BinaryOutputArchive& archive, Scene* scene) override;
		int ReadSegment(cereal::JSONInputArchive& archive, SceneDeserializeDataContainer& data) override;
		int ReadSegment(cereal::BinaryInputArchive& archive, SceneDeserializeDataContainer& data) override;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	struct TransformComponentSerializer : public IEntityComponentSerializer<TransformComponent>
	{
		TransformComponentSerializer() = default;
		TransformComponentSerializer(const TransformComponentSerializer&) = default;
		TransformComponentSerializer& operator=(const TransformComponentSerializer&) = default;
		TransformComponentSerializer(TransformComponentSerializer&&) noexcept = default;
		TransformComponentSerializer& operator=(TransformComponentSerializer&&) noexcept = default;
		~TransformComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};
	
	struct CameraComponentSerializer: public IEntityComponentSerializer<CameraComponent>
	{
		CameraComponentSerializer() = default;
		CameraComponentSerializer(const CameraComponentSerializer&) = default;
		CameraComponentSerializer& operator=(const CameraComponentSerializer&) = default;
		CameraComponentSerializer(CameraComponentSerializer&&) noexcept = default;
		CameraComponentSerializer& operator=(CameraComponentSerializer&&) noexcept = default;
		~CameraComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	struct ScriptComponentSerializer : public IEntityComponentSerializer<ScriptComponent>
	{
		ScriptComponentSerializer() = default;
		ScriptComponentSerializer(const ScriptComponentSerializer&) = default;
		ScriptComponentSerializer& operator=(const ScriptComponentSerializer&) = default;
		ScriptComponentSerializer(ScriptComponentSerializer&&) noexcept = default;
		ScriptComponentSerializer& operator=(ScriptComponentSerializer&&) noexcept = default;
		~ScriptComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};
	
	struct SpriteRendererComponentSerializer: public IEntityComponentSerializer<SpriteRendererComponent>
	{
		SpriteRendererComponentSerializer() = default;
		SpriteRendererComponentSerializer(const SpriteRendererComponentSerializer&) = default;
		SpriteRendererComponentSerializer& operator=(const SpriteRendererComponentSerializer&) = default;
		SpriteRendererComponentSerializer(SpriteRendererComponentSerializer&&) noexcept = default;
		SpriteRendererComponentSerializer& operator=(SpriteRendererComponentSerializer&&) noexcept = default;
		~SpriteRendererComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	struct AudioSourceComponentSerializer : public IEntityComponentSerializer<AudioSourceComponent>
	{
		AudioSourceComponentSerializer() = default;
		AudioSourceComponentSerializer(const AudioSourceComponentSerializer&) = default;
		AudioSourceComponentSerializer& operator=(const AudioSourceComponentSerializer&) = default;
		AudioSourceComponentSerializer(AudioSourceComponentSerializer&&) noexcept = default;
		AudioSourceComponentSerializer& operator=(AudioSourceComponentSerializer&&) noexcept = default;
		~AudioSourceComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	struct VideoPlayerComponentSerializer : public IEntityComponentSerializer<VideoPlayerComponent>
	{
		VideoPlayerComponentSerializer() = default;
		VideoPlayerComponentSerializer(const VideoPlayerComponentSerializer&) = default;
		VideoPlayerComponentSerializer& operator=(const VideoPlayerComponentSerializer&) = default;
		VideoPlayerComponentSerializer(VideoPlayerComponentSerializer&&) noexcept = default;
		VideoPlayerComponentSerializer& operator=(VideoPlayerComponentSerializer&&) noexcept = default;
		~VideoPlayerComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	struct RmlUIDocumentComponentSerializer : public IEntityComponentSerializer<RmlUIDocumentComponent>
	{
		RmlUIDocumentComponentSerializer() = default;
		RmlUIDocumentComponentSerializer(const RmlUIDocumentComponentSerializer&) = default;
		RmlUIDocumentComponentSerializer& operator=(const RmlUIDocumentComponentSerializer&) = default;
		RmlUIDocumentComponentSerializer(RmlUIDocumentComponentSerializer&&) noexcept = default;
		RmlUIDocumentComponentSerializer& operator=(RmlUIDocumentComponentSerializer&&) noexcept = default;
		~RmlUIDocumentComponentSerializer() override = default;

		void AddActorSerializeComponent(Scene* scene, GameActor* actor, VGActorID id) override;
	};

	class SceneSerializer
	{
	public:
		SceneSerializer();
		SceneSerializer(const SceneSerializer&) = default;
		SceneSerializer& operator=(const SceneSerializer&) = default;
		SceneSerializer(SceneSerializer&&) noexcept = default;
		SceneSerializer& operator=(SceneSerializer&&) noexcept = default;
		~SceneSerializer() = default;

		int GetSerializerNumber() const;

		int SerializeScene(cereal::JSONOutputArchive& archive, Scene* scene);
		int DeserializeScene(cereal::JSONInputArchive& archive, Scene* scene);
	private:
		std::unordered_map<String, Ref<ISceneSegmentSerializer>> m_SegmentSerializers;
	};
}
