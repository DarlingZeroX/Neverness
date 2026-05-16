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
#include "NNRuntimeCore/Include/Core/Core.h"
#include <NNKernel/Interface/HSerialization.h>

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

		Ref<ISceneSegmentSerializer> NewRef() override
		{
			return MakeRef<EntitySerializer>();
		}

		String GetSegmentType() override;

		int WriteSegment(cereal::JSONOutputArchive& archive, IScene* scene) override;
		int WriteSegment(cereal::BinaryOutputArchive& archive, IScene* scene) override;
		int ReadSegment(cereal::JSONInputArchive& archive, SceneDeserializeDataContainer& data) override;
		int ReadSegment(cereal::BinaryInputArchive& archive, SceneDeserializeDataContainer& data) override;

		void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) override;
	};

	class SceneSerializer : public ISceneSerializer
	{
	public:
		SceneSerializer();
		SceneSerializer(const SceneSerializer&) = default;
		SceneSerializer& operator=(const SceneSerializer&) = default;
		SceneSerializer(SceneSerializer&&) noexcept = default;
		SceneSerializer& operator=(SceneSerializer&&) noexcept = default;
		~SceneSerializer() override = default;

		void AddSegmentSerializer(const String& segmentType, const Ref<ISceneSegmentSerializer>& serializer) override;
		int GetSerializerNumber() const override;

		int SerializeScene(cereal::JSONOutputArchive& archive, IScene* scene) override;
		int DeserializeScene(cereal::JSONInputArchive& archive, IScene* scene) override;
	private:
		std::unordered_map<String, Ref<ISceneSegmentSerializer>> m_SegmentSerializers;
	};
}
