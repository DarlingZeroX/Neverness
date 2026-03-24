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

#include "SceneSerializer.h"
#include "VGCore/Interface/Loader.h"
#include "HCore/Include/Scene/HBaseComponent.h"

namespace VisionGal
{
	String EntitySerializer::GetSegmentType()
	{
		return String("Entity");
	}

	template<class Archive>
	void WriteEntity(Archive& archive, IScene* scene)
	{
		auto view = scene->GetWorld()->view<Horizon::HEntityObjectData, Horizon::HRelationship>();
		archive(cereal::make_nvp("EntityNumber", view.size_hint()));
		view.each([&](Horizon::HEntityObjectData& entityData, Horizon::HRelationship& relation) { // flecs::entity argument is optional
			archive(cereal::make_nvp("Label", relation.Label));
			archive(cereal::make_nvp("ID", entityData.ID));
			if (relation.Parent == nullptr)
				archive(cereal::make_nvp("Parent", 0));
			else
				archive(cereal::make_nvp("Parent", relation.Parent->GetEntityID()));
			archive(cereal::make_nvp("ComponentIDs", entityData.ComponentIDs));
			});
	}

	int EntitySerializer::WriteSegment(cereal::JSONOutputArchive& archive, IScene* scene)
	{
		WriteEntity(archive, scene);

		return 0;
	}

	int EntitySerializer::WriteSegment(cereal::BinaryOutputArchive& archive, IScene* scene)
	{
		WriteEntity(archive, scene);
		return 0;
	}

	int EntitySerializer::ReadSegment(cereal::JSONInputArchive& archive, SceneDeserializeDataContainer& data)
	{
		int entityNumber = 0;
		archive(entityNumber);
		data.entities.resize(entityNumber);

		for (auto& entity: data.entities)
		{
			archive(entity.Label);
			archive(entity.ID);
			archive(entity.Parent);
			archive(entity.ComponentIDs);
		}

		return 0;
	}

	int EntitySerializer::ReadSegment(cereal::BinaryInputArchive& archive, SceneDeserializeDataContainer& data)
	{
		return 0;
	}

	void EntitySerializer::AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id)
	{
	}

	SceneSerializer::SceneSerializer()
	{
		m_SegmentSerializers[EntitySerializer{}.GetSegmentType()] = MakeRef<EntitySerializer>();
	}

	void SceneSerializer::AddSegmentSerializer(const String& segmentType,
		const Ref<ISceneSegmentSerializer>& serializer)
	{
		m_SegmentSerializers[segmentType] = serializer;
	}

	int SceneSerializer::GetSerializerNumber() const
	{
		return m_SegmentSerializers.size();
	}

	int SceneSerializer::SerializeScene(cereal::JSONOutputArchive& archive, IScene* scene)
	{
		archive(cereal::make_nvp("SegmentNumber", GetSerializerNumber()));

		for (auto& part: m_SegmentSerializers)
		{
			auto* serializer = part.second.get();

			archive(cereal::make_nvp("SegmentType", serializer->GetSegmentType()));
			serializer->WriteSegment(archive, scene);
		}

		return 0;
	}

	int SceneSerializer::DeserializeScene(cereal::JSONInputArchive& archive, IScene* scene)
	{
		int segmentNumber = 0;
		archive(segmentNumber);

		SceneDeserializeDataContainer container;
		container.scene = scene;

		// 反序列化到场景数据容器
		for (int i = 0; i < segmentNumber; i++)
		{
			std::string segmentType;
			archive(segmentType);

			m_SegmentSerializers[segmentType]->ReadSegment(archive, container);
		} 

		// 创建反序列化后的实体
		for (auto& entity: container.entities)
		{
			IGameActor* actor = scene->CreateDeserializeActor(entity);
			auto* entityData = actor->GetComponent<Horizon::HEntityObjectData>();

			for (auto& comID: entity.ComponentIDs)
			{
				auto result = container.componentMap.find(comID);
				if (result != container.componentMap.end())
				{
					//auto* serializer = container.componentMap[comID];
					auto* serializer = result->second;
					entityData->ComponentIDs.push_back(comID);
					entityData->ComponentTypes.push_back(serializer->GetSegmentType());

					serializer->AddActorSerializeComponent(scene, actor, comID);
				}
			}
		}

		scene->UpdateDeserializeActorRelationship();

		return 0;
	}
}
