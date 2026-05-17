/**
 * @file NNRuntimeSceneTickSubsystem.cpp
 * @brief **NNRuntimeSceneTickSubsystem** 实现。
 */

#include "NNRuntimeSceneTickSubsystem.h"

#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"

namespace NN::Runtime::engine
{
NNRuntimeSceneTickSubsystem::NNRuntimeSceneTickSubsystem(Scene::NNRuntimeScene* const scene) noexcept
	: scene_(scene)
{
}

void NNRuntimeSceneTickSubsystem::SetScene(Scene::NNRuntimeScene* const scene) noexcept
{
	scene_ = scene;
}

void NNRuntimeSceneTickSubsystem::Tick(const RuntimeFrameContext& context) noexcept
{
	++tickInvocationCount_;
	if (scene_ != nullptr)
	{
		scene_->TickSystems(context.deltaTimeSeconds);
	}
}

RuntimeTickGroup NNRuntimeSceneTickSubsystem::TickGroup() const noexcept
{
	return RuntimeTickGroup::Update;
}
} // namespace NN::Runtime::engine
