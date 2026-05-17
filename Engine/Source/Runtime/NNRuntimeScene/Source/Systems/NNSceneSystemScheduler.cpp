/**
 * @file NNSceneSystemScheduler.cpp
 * @brief **NNSceneSystemScheduler** 实现。
 */

#include "Systems/NNSceneSystemScheduler.h"

#include <algorithm>

#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
void NNSceneSystemScheduler::Register(ISceneSystem* const system) noexcept
{
	if (system == nullptr)
	{
		return;
	}

	const auto groupIndex = static_cast<std::size_t>(system->TickGroup());
	if (groupIndex >= NNSceneTickGroupCount)
	{
		return;
	}

	auto& bucket = m_Systems[groupIndex];
	if (std::find(bucket.begin(), bucket.end(), system) != bucket.end())
	{
		return;
	}
	bucket.push_back(system);
}

bool NNSceneSystemScheduler::Unregister(ISceneSystem* const system) noexcept
{
	if (system == nullptr)
	{
		return false;
	}

	const auto groupIndex = static_cast<std::size_t>(system->TickGroup());
	if (groupIndex >= NNSceneTickGroupCount)
	{
		return false;
	}

	auto& bucket = m_Systems[groupIndex];
	const auto it = std::find(bucket.begin(), bucket.end(), system);
	if (it == bucket.end())
	{
		return false;
	}
	bucket.erase(it);
	return true;
}

void NNSceneSystemScheduler::Tick(NNRuntimeScene& scene, const float deltaTimeSeconds) noexcept
{
	for (std::size_t group = 0; group < NNSceneTickGroupCount; ++group)
	{
		for (ISceneSystem* const system : m_Systems[group])
		{
			if (system != nullptr)
			{
				system->Tick(scene, deltaTimeSeconds);
			}
		}
	}
}

std::size_t NNSceneSystemScheduler::GetRegisteredCount() const noexcept
{
	std::size_t total = 0;
	for (std::size_t group = 0; group < NNSceneTickGroupCount; ++group)
	{
		total += m_Systems[group].size();
	}
	return total;
}
} // namespace NN::Runtime::Scene
