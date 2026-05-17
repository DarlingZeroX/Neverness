/**
 * @file NNTransformSystem.cpp
 * @brief **NNTransformSystem** 实现。
 */

#include "Systems/NNTransformSystem.h"

#include "Components/NNTransformComponent.h"
#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
void NNTransformSystem::Tick(NNRuntimeScene& scene, const float deltaTimeSeconds) noexcept
{
	(void)deltaTimeSeconds;
	m_LastTickedCount = 0u;
	scene.Query<NNTransformComponent>().Each(
		[this](NNEntity /*handle*/, NNTransformComponent& /*transform*/)
		{
			++m_LastTickedCount;
		});
}
} // namespace NN::Runtime::Scene
