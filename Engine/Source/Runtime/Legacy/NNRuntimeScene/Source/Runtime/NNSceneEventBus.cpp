/**
 * @file NNSceneEventBus.cpp
 * @brief **NNSceneEventBus** 实现。
 */

#include "Runtime/NNSceneEventBus.h"

namespace NN::Runtime::Scene
{
void NNSceneEventBus::Subscribe(NNSceneEventHandler handler)
{
	if (handler)
	{
		m_Handlers.push_back(std::move(handler));
	}
}

void NNSceneEventBus::Emit(const NNSceneEntityEvent& event) const
{
	for (const NNSceneEventHandler& handler : m_Handlers)
	{
		if (handler)
		{
			handler(event);
		}
	}
}

void NNSceneEventBus::Clear() noexcept
{
	m_Handlers.clear();
}
} // namespace NN::Runtime::Scene
