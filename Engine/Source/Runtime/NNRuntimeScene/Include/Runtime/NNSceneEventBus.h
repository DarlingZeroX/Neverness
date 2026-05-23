#pragma once

/**
 * @file NNSceneEventBus.h
 * @brief 场景级事件总线（替代 Legacy 组件 virtual OnXXX 钩子）。
 */

#include <cstdint>
#include <functional>
#include <vector>

#include "../Reflection/NNComponentTypeId.h"
#include "../Scene/NNEntity.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief 场景事件类型（Phase 2 首包）。 */
	enum class NNSceneEventType : std::uint8_t
	{
		EntityCreated = 0,
		EntityDestroyed,
		ParentChanged,
		ComponentEmplaced,
	};

	struct NN_RUNTIME_SCENE_API NNSceneEntityEvent
	{
		NNSceneEventType Type = NNSceneEventType::EntityCreated;
		NNEntity Entity = NNEntityInvalid;
		NNEntity OtherEntity = NNEntityInvalid;
		NNComponentTypeId ComponentTypeId = NNComponentTypeIdInvalid;
	};

	using NNSceneEventHandler = std::function<void(const NNSceneEntityEvent&)>;

	/**
	 * @brief 单线程场景事件派发器；订阅方由工具链 / Gameplay 桥接层注册。
	 */
	class NN_RUNTIME_SCENE_API NNSceneEventBus
	{
	public:
		void Subscribe(NNSceneEventHandler handler);

		void Emit(const NNSceneEntityEvent& event) const;

		void Clear() noexcept;

		[[nodiscard]] std::size_t GetSubscriberCount() const noexcept { return m_Handlers.size(); }

	private:
		std::vector<NNSceneEventHandler> m_Handlers{};
	};
} // namespace NN::Runtime::Scene
