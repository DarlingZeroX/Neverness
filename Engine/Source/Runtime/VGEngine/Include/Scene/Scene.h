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
#include "../EngineConfig.h"
#include "VGCore/Interface/SceneInterface.h"
#include <memory>
#include <unordered_map>
#include <NNKernel/Interface/HRandom.h>
#include <NNKernel/Include/Scene/HBaseComponent.h>
#include <NNKernel/Include/Event/HEventDelegate.h>

namespace VisionGal
{
	class VG_ENGINE_API Scene: public IScene
	{
	public:
		Scene();
		Scene(const Scene&) = default;
		Scene& operator=(const Scene&) = default;
		Scene(Scene&&) noexcept = default;
		Scene& operator=(Scene&&) noexcept = default;
		~Scene() override = default;

		//Horizon::HEventDelegate<const Horizon::HUISceneEvent&> OnSceneEvent;
		IGameActor* CreateActor(IEntity* parent = nullptr) override;

		template<typename T, class = typename std::enable_if<std::is_base_of<IComponent, T>::value>::type>
		T* AddEntityComponent(IEntity* entity)
		{
			T& com = GetWorld()->emplace<T>(entity->GetEntity());

			AddEntityComponent(entity, &com);

			return &com;
		}
		void AddEntityComponent(IEntity* entity, IComponent* component) override;
		 
		// 通过 IScene 继承
		IEntity* GetActor(VGActorID entityID) override;
		bool RemoveActor(VGActorID entityID) override;
		bool ExistActor(VGActorID entityID) override;
		Horizon::HECS* GetWorld() override;

		void Update() override;
		IGameActor* GetSceneActor() override;

		IGameActor* CreateDeserializeActor(const SceneDeserializeEntity& entity) override;
		void UpdateDeserializeComponent(IEntity* entity, IComponent* component) override;
		void UpdateDeserializeActorRelationship() override;
	private:

		VGActorID NewEntityID();
		VGComponentID NewComponentID();
		void SetEntityBaseData(IEntity* entity, VGActorID id, VGActorID parentID, const std::string& lable);
		void CreateSceneActor();
		std::string GetNewLabel(IEntity* hObj);
	private:
		std::unordered_map<VGActorID, std::shared_ptr<IGameActor>> m_IDEntityMap;
		std::unordered_set<VGComponentID> m_EntityComponentID;
		ECS m_Registry;

		std::shared_ptr<IGameActor> m_SceneActor;
		std::string m_SceneName = "Scene";
	};
}
