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
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"
#include <NNCore/Include/Scene/HEntityInterface.h>
#include <NNCore/Include/Scene/HComponentInterface.h>
#include <NNCore/Interface/HSerialization.h>

namespace NN::Runtime
{
	class IScene;

	using ECS = NN::Core::HECS;

	struct IEntity : public NN::Core::HEntityInterface
	{
		using Entity = entt::entity;
		friend class SceneLegacy;

		IEntity() = default;
		IEntity(const IEntity&) = default;
		IEntity& operator=(const IEntity&) = default;
		IEntity(IEntity&&) noexcept = default;
		IEntity& operator=(IEntity&&) noexcept = default;
		~IEntity() override = default;

		virtual void Initialize(IScene* scene) {};
		virtual IEntity* Instantiate() { return nullptr; };
	protected:
		void SetEntityID(size_t id) { m_EntityID = id; }
		void SetBaseScene(NN::Core::HSceneInterface* scene) { m_BaseScene = scene; }
		void SetBaseEntity(const Entity& entt) { m_BaseEntity = entt; }
	};
	 
	struct IPrimitiveEntity : public IEntity
	{
		using Entity = entt::entity;

		IPrimitiveEntity() = default;
		IPrimitiveEntity(const IPrimitiveEntity&) = default;
		IPrimitiveEntity& operator=(const IPrimitiveEntity&) = default;
		IPrimitiveEntity(IPrimitiveEntity&&) noexcept = default;
		IPrimitiveEntity& operator=(IPrimitiveEntity&&) noexcept = default;
		~IPrimitiveEntity() override = default;

		virtual IPrimitiveEntity* InstantiatePrimitive() { return nullptr; };
		IPrimitiveEntity* Instantiate() override { return InstantiatePrimitive(); };
	};

	struct IComponent : public NN::Core::HComponentInterface
	{
		uint64 EntityComID = 0;
	public:
		IComponent() = default;
		IComponent(const IComponent&) = default;
		IComponent& operator=(const IComponent&) = default;
		IComponent(IComponent&&) noexcept = default;
		IComponent& operator=(IComponent&&) noexcept = default;
		~IComponent() override = default;

		virtual std::string GetComponentType() const = 0;

		template <class Archive>
		void serializeIComponent(Archive& ar) {
			ar(cereal::make_nvp("EntityComponentID", EntityComID));
		}

		template <class Archive>
		void saveIComponent(Archive& ar) const {
			ar(cereal::make_nvp("EntityComponentID", EntityComID));
		}

		template <class Archive>
		void loadIComponent(Archive& ar) {
			ar(EntityComID);
		}
	private:
		std::string ComponentType;
		//virtual IComponent* Emplace(NN::Core::HSceneInterface* scene, IEntity* entity) = 0;
	};

	//struct IComponentDrawer
	//{
	//	virtual ~IComponentDrawer() {}
	//
	//	virtual void OnGUI(IEntity* entity) = 0;
	//	virtual std::string GetBindComponentType() = 0;
	//};

	class NN_RUNTIME_CORE_API IGameActor : public IEntity
	{
	public:
		IGameActor() = default;
		IGameActor(const IGameActor&) = default;
		IGameActor& operator=(const IGameActor&) = default;
		IGameActor(IGameActor&&) noexcept = default;
		IGameActor& operator=(IGameActor&&) noexcept = default;
		~IGameActor() override = default;

		template<typename T>
		T* AddComponent();

		virtual void SetLabel(const String& label);
		virtual String GetLabel();
		virtual void SetVisible(bool visible)  = 0;
		virtual bool GetVisible()  = 0;

		virtual IComponent* GetComponentByType(const String& type) = 0;
		virtual IComponent* AddComponentByType(const String& type)  = 0;
	public:
		void Initialize(IScene* scene) override {};
	};

	struct SceneDeserializeEntity
	{
		String Label;
		VGActorID ID;
		VGActorID Parent;
		std::vector<uint64> ComponentIDs;
	};

	struct IScene : public VGEngineResource,public NN::Core::HSceneInterface
	{
		IScene() = default;
		IScene(const IScene&) = default;
		IScene& operator=(const IScene&) = default;
		IScene(IScene&&) noexcept = default;
		IScene& operator=(IScene&&) noexcept = default;
		~IScene() override = default;

		virtual IGameActor* CreateActor(IEntity* parent = nullptr) = 0;
		virtual void AddEntityComponent(IEntity* entity, IComponent* component) = 0;

		virtual IEntity* GetActor(VGActorID entityID) = 0;
		virtual bool RemoveActor(VGActorID entityID) = 0;
		virtual bool ExistActor(VGActorID entityID) = 0;

		virtual void Update() = 0;
		virtual IGameActor* GetSceneActor() = 0;

		virtual IGameActor* CreateDeserializeActor(const SceneDeserializeEntity& entity) = 0;
		virtual void UpdateDeserializeComponent(IEntity* entity, IComponent* component) = 0;
		virtual void UpdateDeserializeActorRelationship() = 0;
	};

	template <typename T>
	T* IGameActor::AddComponent()
	{
		IScene* scene = dynamic_cast<IScene*>(m_BaseScene);
		T& com = GetWorld()->emplace<T>(GetEntity());
		scene->AddEntityComponent(this, &com);

		return &com;
	}
}
