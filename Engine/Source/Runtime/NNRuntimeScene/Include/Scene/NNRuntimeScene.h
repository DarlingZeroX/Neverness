#pragma once

/**
 * @file NNRuntimeScene.h
 * @brief ECS-first 运行时场景：Registry + 世代 Handle + System 调度 + 事件/脏跟踪（Phase 2+）。
 *
 * 职责：实体生命周期、POD 组件、Query、组件元数据、System Tick、层级 API。
 * 不负责：Legacy IGameActor、VGNativeEngineAPI 扩展、Prefab/Streaming 完整实现。
 *
 * 线程契约：Phase 2 假定单线程；由宿主在 game loop 同一线程调用。
 */

#include "Components/NNRelationshipComponent.h"
#include "Components/NNTagComponent.h"
#include "Components/NNTransformComponent.h"
#include "Reflection/NNComponentRegistry.h"
#include "Runtime/NNDirtyTracker.h"
#include "Runtime/NNSceneEventBus.h"
#include "Scene/NNEntityHandle.h"
#include "Scene/NNWorld.h"
#include "Systems/NNHierarchySystem.h"
#include "Systems/NNSceneSystemScheduler.h"
#include "Systems/NNSceneUpdateSystem.h"
#include "Systems/NNTransformSystem.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

#include <vector>

namespace NN::Runtime::Scene
{
	template <typename... Components>
	class NNEntityQuery;

	/**
	 * @brief 新一代运行时场景世界（数据导向；实体不是 C++ 对象）。
	 */
	class NN_RUNTIME_SCENE_API NNRuntimeScene
	{
	public:
		NNRuntimeScene();
		~NNRuntimeScene();

		NNRuntimeScene(const NNRuntimeScene&) = delete;
		NNRuntimeScene& operator=(const NNRuntimeScene&) = delete;

		[[nodiscard]] NNEntity CreateEntity() noexcept;

		/** @brief 创建实体并挂载默认 Transform / Relationship / Tag 组件。 */
		[[nodiscard]] NNEntity CreateEntityWithDefaults() noexcept;

		bool DestroyEntity(NNEntity handle) noexcept;

		[[nodiscard]] bool IsAlive(NNEntity handle) const noexcept;

		[[nodiscard]] NNWorld& GetRegistry() noexcept { return m_Registry; }
		[[nodiscard]] const NNWorld& GetRegistry() const noexcept { return m_Registry; }

		[[nodiscard]] NNComponentRegistry& GetComponentRegistry() noexcept { return m_ComponentRegistry; }
		[[nodiscard]] const NNComponentRegistry& GetComponentRegistry() const noexcept
		{
			return m_ComponentRegistry;
		}

		[[nodiscard]] NNSceneSystemScheduler& GetSystemScheduler() noexcept { return m_SystemScheduler; }
		[[nodiscard]] const NNSceneSystemScheduler& GetSystemScheduler() const noexcept
		{
			return m_SystemScheduler;
		}

		void RegisterSystem(ISceneSystem* system) noexcept;

		void TickSystems(float deltaTimeSeconds) noexcept;

		[[nodiscard]] NNSceneEventBus& GetEventBus() noexcept { return m_EventBus; }
		[[nodiscard]] const NNSceneEventBus& GetEventBus() const noexcept { return m_EventBus; }

		[[nodiscard]] NNDirtyTracker& GetDirtyTracker() noexcept { return m_DirtyTracker; }
		[[nodiscard]] const NNDirtyTracker& GetDirtyTracker() const noexcept { return m_DirtyTracker; }

		[[nodiscard]] NNHierarchySystem& GetHierarchySystem() noexcept { return m_HierarchySystem; }
		[[nodiscard]] NNTransformSystem& GetTransformSystem() noexcept { return m_TransformSystem; }

		bool SetParent(NNEntity child, NNEntity parent) noexcept;

		[[nodiscard]] NNEntity GetParent(NNEntity entity) const noexcept;

		[[nodiscard]] std::vector<NNEntity> GetChildren(NNEntity entity) const;

		template <typename T, typename... Args>
		T& Emplace(NNEntity handle, Args&&... args)
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			T& component = m_Registry.emplace<T>(enttEntity, std::forward<Args>(args)...);
			NotifyComponentEmplaced(handle, component);
			return component;
		}

		template <typename T>
		[[nodiscard]] T* TryGet(NNEntity handle) noexcept
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			if (enttEntity == entt::null)
			{
				return nullptr;
			}
			return m_Registry.try_get<T>(enttEntity);
		}

		template <typename T>
		[[nodiscard]] const T* TryGet(NNEntity handle) const noexcept
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			if (enttEntity == entt::null)
			{
				return nullptr;
			}
			return m_Registry.try_get<T>(enttEntity);
		}

		template <typename T>
		[[nodiscard]] bool Has(NNEntity handle) const noexcept
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			if (enttEntity == entt::null)
			{
				return false;
			}
			return m_Registry.all_of<T>(enttEntity);
		}

		template <typename T>
		bool Remove(NNEntity handle) noexcept
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			if (enttEntity == entt::null)
			{
				return false;
			}
			if (!m_Registry.all_of<T>(enttEntity))
			{
				return false;
			}
			m_Registry.remove<T>(enttEntity);
			const NNComponentTypeId typeId = m_ComponentRegistry.FindTypeId(std::type_index(typeid(T)));
			m_DirtyTracker.MarkComponentDirty(handle, typeId);
			return true;
		}

		template <typename... Components>
		[[nodiscard]] NNEntityQuery<Components...> Query() noexcept
		{
			return NNEntityQuery<Components...>(*this);
		}

		[[nodiscard]] NNEntity HandleFromEntt(entt::entity entity) const noexcept;

		/** @brief 遍历存活实体（序列化、调试）。 */
		template <typename Func>
		void ForEachAliveEntity(Func&& func) const
		{
			m_EntityTable.ForEachAlive(std::forward<Func>(func));
		}

	private:
		void RegisterBuiltinComponents();
		void RegisterDefaultSystems();

		template <typename T>
		void NotifyComponentEmplaced(NNEntity handle, const T&)
		{
			const NNComponentTypeId typeId = m_ComponentRegistry.FindTypeId(std::type_index(typeid(T)));
			NNSceneEntityEvent evt{};
			evt.Type = NNSceneEventType::ComponentEmplaced;
			evt.Entity = handle;
			evt.ComponentTypeId = typeId;
			m_EventBus.Emit(evt);
			m_DirtyTracker.MarkComponentDirty(handle, typeId);
		}

		NNWorld m_Registry{};
		NNEntityHandleTable m_EntityTable{};
		NNComponentRegistry m_ComponentRegistry{};
		NNSceneSystemScheduler m_SystemScheduler{};
		NNSceneEventBus m_EventBus{};
		NNDirtyTracker m_DirtyTracker{};
		NNHierarchySystem m_HierarchySystem{};
		NNTransformSystem m_TransformSystem{};
		NNSceneUpdateSystem m_SceneUpdateSystem{};
	};
} // namespace NN::Runtime::Scene

#include "Query/NNEntityQuery.h"
