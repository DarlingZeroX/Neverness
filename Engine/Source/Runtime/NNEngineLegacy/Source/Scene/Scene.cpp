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

#include "Scene/Scene.h"
#include "Scene/Components.h"
#include <NNCore/Include/Scene/HSceneHierachy.h>
#include "Scene/GameActor.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"

namespace NN::Runtime
{
	constexpr VGActorID ConstHUISceneActor = 0;

	class SceneActor : public GameActor
	{
	public:
		SceneActor() = default;
		SceneActor(const SceneActor&) = default;
		SceneActor& operator=(const SceneActor&) = default;
		SceneActor(SceneActor&&) noexcept = default;
		SceneActor& operator=(SceneActor&&) noexcept = default;
		~SceneActor() override = default;

		void Initialize(IScene* scene) override {  };
	};

	void Scene::Update()
	{
	}
	 

	NN::Core::HECS* Scene::GetWorld()
	{
		return &m_Registry;
	}

	IGameActor* Scene::GetSceneActor()
	{
		return m_SceneActor.get();
	}


	VGActorID Scene::NewEntityID()
	{
		srand(std::clock());

		NN::Core::HRandom randEngine;
		randEngine.Seed(rand());

		auto result = randEngine.NextUINT64();

		while (m_IDEntityMap.find(result) != m_IDEntityMap.end())
		{
			result = randEngine.NextUINT64();
		}

		return result;
	}

	VGComponentID Scene::NewComponentID()
	{
		srand(std::clock());

		NN::Core::HRandom randEngine;
		randEngine.Seed(rand());

		auto result = randEngine.NextUINT64();

		while (m_EntityComponentID.find(result) != m_EntityComponentID.end())
		{
			result = randEngine.NextUINT64();
		}

		return result;
	}

	void Scene::SetEntityBaseData(IEntity* entity, VGActorID id, VGActorID parentID, const std::string& lable)
	{
		entity->SetEntityID(id); 
		entity->SetBaseScene(this);

		// Entity data
		auto& data = m_Registry.emplace_or_replace<NN::Core::HEntityObjectData>(entity->GetEntity());
		auto& relationship = m_Registry.emplace_or_replace<NN::Core::HRelationship>(entity->GetEntity());
		data.ID = id;
		data.Entity = entity;
		relationship.Label = lable;
		relationship.ParentID = parentID;
	}

	IGameActor* Scene::CreateDeserializeActor(const SceneDeserializeEntity& deEntity)
	{
		if (deEntity.ID == ConstHUISceneActor)
			return GetSceneActor();

		auto entity = MakeRef<GameActor>();
		entity->SetBaseEntity(this->GetWorld()->create());

		{
			SetEntityBaseData(entity.get(), deEntity.ID, deEntity.Parent, deEntity.Label);
			m_IDEntityMap[deEntity.ID] = entity;
		}

		return entity.get();
	}

	void Scene::UpdateDeserializeComponent(IEntity* entity, IComponent* component)
	{
		m_EntityComponentID.insert(component->EntityComID);
		component->SetOwner(entity);
	}

	void Scene::UpdateDeserializeActorRelationship()
	{
		auto view = GetWorld()->view<NN::Core::HEntityObjectData, NN::Core::HRelationship>();

		view.each([&, this](NN::Core::HEntityObjectData& data, NN::Core::HRelationship& relationship)
			{
				if (data.ID == ConstHUISceneActor)
					return;

				NN::Core::HHierarchy::SetParent(
					m_Registry,
					*data.Entity,
					m_IDEntityMap[relationship.ParentID].get()
				);

			});
	}

	IGameActor* Scene::CreateActor(IEntity* parent)
	{
		auto entity = MakeRef<GameActor>();
		entity->SetBaseEntity(this->GetWorld()->create());

		{
			auto ID = NewEntityID();
			IEntity* realParent = parent == nullptr ? m_SceneActor.get() : parent;

			SetEntityBaseData(entity.get(), ID, realParent->GetEntityID(), GetNewLabel(entity.get()));
			NN::Core::HHierarchy::SetParent(m_Registry, *entity, realParent);

			m_IDEntityMap[ID] = entity;				//map entity id to entity object
		}

		entity->Initialize(this);
		entity->AddComponent<TransformComponent>();

		// 事件
		SceneEvent evt;
		evt.EventType = SceneEventType::ActorCreating;
		evt.Actor = entity.get();
		EngineEventBus::Get().OnSceneEvent.Invoke(evt);

		return entity.get();
	}

	void Scene::CreateSceneActor()
	{
		m_SceneActor = MakeRef<SceneActor>();
		m_SceneActor->Initialize(this);
		m_SceneActor->SetEntityID(ConstHUISceneActor);
		m_SceneActor->SetBaseEntity(this->GetWorld()->create());
		m_SceneActor->SetBaseScene(this);

		auto& relationship = m_Registry.emplace<NN::Core::HRelationship>(m_SceneActor->GetEntity());

		auto& object = m_Registry.emplace<NN::Core::HEntityObjectData>(m_SceneActor->GetEntity());
		object.Entity = m_SceneActor.get();
		object.ID = ConstHUISceneActor;
		relationship.Label = m_SceneName;

		m_IDEntityMap[ConstHUISceneActor] = m_SceneActor;
	}

	std::string Scene::GetNewLabel(IEntity* hObj)
	{
		return std::string("New Label");
	}

	Scene::Scene()
	{
		CreateSceneActor();
	}

	void Scene::AddEntityComponent(IEntity* entity, IComponent* component)
	{
		uint64 comID = NewComponentID();

		NN::Core::HEntityObjectData* entt_obj = entity->GetComponent<NN::Core::HEntityObjectData>();
		entt_obj->ComponentTypes.push_back(component->GetComponentType());
		entt_obj->ComponentIDs.push_back(comID);

		component->SetOwner(entity);
		component->EntityComID = comID;
		component->Initialize();

		m_EntityComponentID.insert(comID);
	}

	IEntity* Scene::GetActor(VGActorID entityID)
	{
		if (auto result = m_IDEntityMap.find(entityID); result != m_IDEntityMap.end())
		{
			return result->second.get();
		}
		return nullptr;
	}

	bool Scene::RemoveActor(VGActorID entityID)
	{
		if (auto result = m_IDEntityMap.find(entityID); result != m_IDEntityMap.end())
		{
			IGameActor& entity = *result->second.get();
			auto& relation = *entity.GetComponent<NN::Core::HRelationship>();

			NN::Core::HHierarchy::DisconnectParent(m_Registry, entity, relation);

			m_Registry.destroy(entity.GetEntity());

			// 事件
			SceneEvent evt;
			evt.EventType = SceneEventType::ActorRemoved;
			evt.Actor = &entity;
			evt.ActorID = entity.GetEntityID();
			EngineEventBus::Get().OnSceneEvent.Invoke(evt);

			m_IDEntityMap.erase(entityID);

			return 0;
		}

		return 0;
	}

	bool Scene::ExistActor(VGActorID entityID)
	{
		return m_IDEntityMap.find(entityID) != m_IDEntityMap.end();
	}

}
