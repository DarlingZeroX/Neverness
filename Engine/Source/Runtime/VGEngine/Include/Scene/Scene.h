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
#include "../Interface/SceneInterface.h"
#include <memory>
#include <unordered_map>
#include <HCore/Interface/HRandom.h>
#include <HCore/Include/Scene/HBaseComponent.h>
#include <HCore/Include/Event/HEventDelegate.h>

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
		GameActor* CreateActor(IEntity* parent = nullptr) override;

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

		void Update();
		GameActor* GetSceneActor();

		GameActor* CreateDeserializeActor(const SceneDeserializeEntity& entity);
		void UpdateDeserializeComponent(IEntity* entity, IComponent* component);
		void UpdateDeserializeActorRelationship();
	private:

		VGActorID NewEntityID();
		VGComponentID NewComponentID();
		void SetEntityBaseData(IEntity* entity, VGActorID id, VGActorID parentID, const std::string& lable);
		void CreateSceneActor();
		std::string GetNewLabel(IEntity* hObj);
	private:
		std::unordered_map<VGActorID, std::shared_ptr<GameActor>> m_IDEntityMap;
		std::unordered_set<VGComponentID> m_EntityComponentID;
		ECS m_Registry;

		std::shared_ptr<GameActor> m_SceneActor;
		std::string m_SceneName = "Scene";
	};
}
