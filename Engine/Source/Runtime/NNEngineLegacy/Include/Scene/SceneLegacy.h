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
#include "NNRuntimeCore/Interface/SceneInterface.h"
#include <memory>
#include <unordered_map>
#include <NNCore/Interface/HRandom.h>
#include <NNCore/Include/Scene/HBaseComponent.h>
#include <NNCore/Include/Event/HEventDelegate.h>

namespace NN::Runtime
{
	class VG_ENGINE_API SceneLegacy: public IScene
	{
	public:
		SceneLegacy();
		SceneLegacy(const SceneLegacy&) = default;
		SceneLegacy& operator=(const SceneLegacy&) = default;
		SceneLegacy(SceneLegacy&&) noexcept = default;
		SceneLegacy& operator=(SceneLegacy&&) noexcept = default;
		~SceneLegacy() override = default;

		//NN::Core::HEventDelegate<const NN::Core::HUISceneEvent&> OnSceneEvent;
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
		NN::Core::HECS* GetWorld() override;

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
