/**
 * @file NNSceneUpdateSystem.cpp
 * @brief **NNSceneUpdateSystem** 实现。
 */

#include "Systems/NNSceneUpdateSystem.h"

#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
void NNSceneUpdateSystem::Tick(NNRuntimeScene& scene, const float deltaTimeSeconds) noexcept
{
	(void)scene;
	(void)deltaTimeSeconds;
	++m_FrameCounter;
}
} // namespace NN::Runtime::Scene
