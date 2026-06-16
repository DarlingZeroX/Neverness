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
#include "NNRuntimeAssetLegacy/Include/SceneAsset.h"
//#include "../../Scene/Components.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include <NNCore/Interface/HSerialization.h>

namespace NN::Runtime
{
	struct ISceneSegmentSerializer;

	struct SceneDeserializeDataContainer
	{
		IScene* scene;
		std::vector<SceneDeserializeEntity> entities;
		std::unordered_map<VGActorID, ISceneSegmentSerializer*> componentMap;
	};

	struct ISceneSegmentSerializer
	{
		virtual ~ISceneSegmentSerializer() = default;

		virtual Ref<ISceneSegmentSerializer> NewRef() { H_ASSERT_NOT_NULL(nullptr); return nullptr; };

		// 获取场景序列化段的类型,序列化后的文件是一段一段的,包含Entity, TransformComponent, CameraComponent....等段.		
		virtual String GetSegmentType() = 0;

		// 把当前段类型的数据序列化,比如Entity, TransformComponent, CameraComponent....等段
		virtual int WriteSegment(cereal::JSONOutputArchive& archive, IScene* scene) = 0;
		virtual int WriteSegment(cereal::BinaryOutputArchive& archive, IScene* scene) = 0;

		virtual int ReadSegment(cereal::JSONInputArchive& archive, SceneDeserializeDataContainer& data) = 0;
		virtual int ReadSegment(cereal::BinaryInputArchive& archive, SceneDeserializeDataContainer& data) = 0;

		virtual void AddActorSerializeComponent(IScene* scene, IGameActor* actor, VGActorID id) = 0;
	};
	  
	/// <summary>
	///	实体组件专用的序列化器
	/// </summary>
	/// <typeparam name="T">Entity Component实体组件的类型</typeparam>
	template<class T, class = typename std::enable_if<std::is_base_of<IComponent, T>::value>::type>
	struct IEntityComponentSerializer: ISceneSegmentSerializer
	{
		~IEntityComponentSerializer() override = default;

		String GetSegmentType() override
		{
			static T t;
			return t.GetComponentType();
		};

		template<class Archive>
		void WriteComponentTemplate(Archive& archive, IScene* scene)
		{
			T componentTemp;
			std::string componentType = componentTemp.GetComponentType();
			auto view = scene->GetWorld()->view<T>();
			archive(cereal::make_nvp("ComponentNumber", view.size()));
			view.each([&](T& component) { // flecs::entity argument is optional
				//std::string componentId = std::to_string(component.EntityComID);
				archive(cereal::make_nvp(componentType, component));
				});
		}

		int WriteSegment(cereal::JSONOutputArchive& archive, IScene* scene) override
		{
			WriteComponentTemplate(archive, scene);
			return 0;
		}

		int WriteSegment(cereal::BinaryOutputArchive& archive, IScene* scene) override
		{
			WriteComponentTemplate(archive, scene);
			return 0;
		}

		int ReadSegment(cereal::JSONInputArchive& archive, SceneDeserializeDataContainer& data) override
		{
			int componentNumber = 0;
			archive(componentNumber);

			for (int i = 0; i< componentNumber; i ++)
			{
				T component;
				archive(component);
				data.componentMap[component.EntityComID] = this;
				m_ComponentMap[component.EntityComID] = component;
			}

			return 0;
		}

		int ReadSegment(cereal::BinaryInputArchive& archive, SceneDeserializeDataContainer& data) override
		{
			return 0;
		}

		std::unordered_map<VGActorID, T> m_ComponentMap;
	};

	class ISceneSerializer
	{
	public:
		virtual ~ISceneSerializer() = default;

		virtual int GetSerializerNumber() const = 0;
		virtual void AddSegmentSerializer(const String& segmentType, const Ref<ISceneSegmentSerializer>& serializer) = 0;

		virtual int SerializeScene(cereal::JSONOutputArchive& archive, IScene* scene) = 0;
		virtual int DeserializeScene(cereal::JSONInputArchive& archive, IScene* scene) = 0;
	};
}
