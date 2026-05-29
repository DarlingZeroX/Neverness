#pragma once

/**
 * @file NNRuntimeScene.h
 * @brief ECS-first 运行时场景：Registry + 世代 Handle + System 调度 + 事件/脏跟踪（Phase 2+）。
 *
 * 职责：实体生命周期、POD 组件、Query、组件元数据、System Tick、层级 API。
 * 不负责：Legacy IGameActor、NNNativeEngineAPI 扩展、Prefab/Streaming 完整实现。
 *
 * 线程契约：Phase 2 假定单线程；由宿主在 game loop 同一线程调用。
 */

#include "../Components/NNAudioSourceComponent.h"
#include "../Components/NNCameraComponent.h"
#include "../Components/NNRelationshipComponent.h"
#include "../Components/NNSpriteRendererComponent.h"
#include "../Components/NNTagComponent.h"
#include "../Components/NNTransformComponent.h"
#include "../Components/NNVideoPlayerComponent.h"
#include "../Reflection/NNComponentRegistry.h"
#include "../Runtime/NNDirtyTracker.h"
#include "../Runtime/NNSceneEventBus.h"
#include "../Scene/NNEntityHandle.h"
#include "../Scene/NNWorld.h"
#include "../Systems/NNHierarchySystem.h"
#include "../Systems/NNSceneSystemScheduler.h"
#include "../Systems/NNSceneUpdateSystem.h"
#include "../Systems/NNTransformSystem.h"
#include "../Systems/NNCameraSystem.h"
#include "../../NNRuntimeSceneExport.h"

#include <atomic>
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

		[[nodiscard]] NNEntityHandleTable& GetHandleTable() noexcept { return m_EntityTable; }
		[[nodiscard]] const NNEntityHandleTable& GetHandleTable() const noexcept { return m_EntityTable; }

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
		[[nodiscard]] NNCameraSystem& GetCameraSystem() noexcept { return m_CameraSystem; }

		bool SetParent(NNEntity child, NNEntity parent) noexcept;

		[[nodiscard]] NNEntity GetParent(NNEntity entity) const noexcept;

		[[nodiscard]] std::vector<NNEntity> GetChildren(NNEntity entity) const;

		template <typename T, typename... Args>
		T& Emplace(NNEntity handle, Args&&... args)
		{
			const entt::entity enttEntity = m_EntityTable.Resolve(handle);
			T& component = m_Registry.emplace<T>(enttEntity, std::forward<Args>(args)...);
			NotifyComponentEmplaced(handle, component);
			IncrementReflectionVersion();
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
			IncrementReflectionVersion();
			return true;
		}

		template <typename... Components>
		[[nodiscard]] NNEntityQuery<Components...> Query() noexcept
		{
			return NNEntityQuery<Components...>(*this);
		}

		/**
		 * @brief 绑定组件类型的 Runtime Access 函数指针到注册表描述符。
		 *
		 * 此模板方法在 NNRuntimeScene 完整定义可见处声明，
		 * 可安全访问 TryGet<T>/Has<T>/Emplace<T>/Remove<T> 等模板方法，
		 * 避免 NN_REGISTER_COMPONENT 宏中的循环包含问题。
		 *
		 * 调用时机：RegisterBuiltinComponents() 或插件初始化时。
		 * 与 NN_REGISTER_COMPONENT 宏的配合：宏注册字段元数据，此方法补充函数指针。
		 */
		template <typename T>
		void BindComponentType(const char* nameUtf8)
		{
			NNComponentTypeDesc desc{};
			desc.TypeIndex = std::type_index(typeid(T));
			desc.NameUtf8 = nameUtf8;
			desc.NameHash = fnv1a_64(nameUtf8);
			desc.TypeId = desc.NameHash;
			desc.SizeBytes = sizeof(T);

			// Runtime Type-Erased ECS Access 函数指针
			desc.GetComponentPtrFn = [](NNRuntimeScene* s, NNEntity e) -> void* {
				return s->TryGet<T>(e);
			};
			desc.GetComponentConstPtrFn = [](const NNRuntimeScene* s, NNEntity e) -> const void* {
				return s->TryGet<T>(e);
			};
			desc.HasComponentFn = [](const NNRuntimeScene* s, NNEntity e) -> bool {
				return s->Has<T>(e);
			};
			desc.AddComponentFn = [](NNRuntimeScene* s, NNEntity e) -> bool {
				if (s->Has<T>(e)) return false;
				(void)s->Emplace<T>(e);
				return true;
			};
			desc.RemoveComponentFn = [](NNRuntimeScene* s, NNEntity e) -> bool {
				return s->Remove<T>(e);
			};
			desc.ForEachEntityFn = [](NNRuntimeScene* s,
				void(*cb)(NNEntity, void*, void*), void* ud) {
				auto view = s->GetRegistry().view<T>();
				for (auto enttEnt : view)
				{
					NNEntity h = s->HandleFromEntt(enttEnt);
					void* p = s->TryGet<T>(h);
					if (p) cb(h, p, ud);
				}
			};

			m_ComponentRegistry.RegisterTypeWithFields(desc);
		}

		[[nodiscard]] NNEntity HandleFromEntt(entt::entity entity) const noexcept;

		// ── Editor Snapshot 版本号 + 增量脏条目 ──

		/** @brief 层级版本号——Entity 创建/销毁/Parent 变更时递增。 */
		[[nodiscard]] std::uint64_t HierarchyVersion() const noexcept
		{
			return m_HierarchyVersion.load(std::memory_order_relaxed);
		}

		void IncrementHierarchyVersion() noexcept
		{
			m_HierarchyVersion.fetch_add(1u, std::memory_order_relaxed);
		}

		/** @brief 增量脏条目——层级变化时追加，全量快照拉取后清空。 */
		struct DirtyHierarchyEntry
		{
			NNEntity   entity{NNEntityInvalid};
			std::uint32_t changeFlags{0};
		};

		void MarkHierarchyDirty(NNEntity entity, std::uint32_t changeFlags) noexcept;

		[[nodiscard]] const std::vector<DirtyHierarchyEntry>& GetDirtyHierarchyEntries() const noexcept
		{
			return m_DirtyHierarchyEntries;
		}

		void ClearDirtyHierarchyEntries() noexcept
		{
			m_DirtyHierarchyEntries.clear();
		}

		/** @brief Transform 版本号——NNTransformSystem 计算 WorldMatrix 时递增。 */
		[[nodiscard]] std::uint64_t TransformVersion() const noexcept
		{
			return m_TransformVersion.load(std::memory_order_relaxed);
		}

		void IncrementTransformVersion() noexcept
		{
			m_TransformVersion.fetch_add(1u, std::memory_order_relaxed);
		}

		/** @brief Reflection 版本号——组件增删时递增，C# 每帧 poll 以决定是否重建 Inspector 缓存。 */
		[[nodiscard]] std::uint64_t ReflectionVersion() const noexcept
		{
			return m_ReflectionVersion.load(std::memory_order_relaxed);
		}

		void IncrementReflectionVersion() noexcept
		{
			m_ReflectionVersion.fetch_add(1u, std::memory_order_relaxed);
		}

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
		NNCameraSystem m_CameraSystem{};

		// ── Editor Snapshot 版本号 + 增量脏条目 ──
		std::atomic<std::uint64_t> m_HierarchyVersion{0u};
		std::atomic<std::uint64_t> m_TransformVersion{0u};
		std::atomic<std::uint64_t> m_ReflectionVersion{0u};
		std::vector<DirtyHierarchyEntry> m_DirtyHierarchyEntries{};
	};
} // namespace NN::Runtime::Scene

#include "Query/NNEntityQuery.h"
