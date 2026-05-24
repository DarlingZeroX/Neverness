/**
 * @file NNCameraSystem.cpp
 * @brief NNCameraSystem 实现：遍历所有 NNCameraComponent，计算投影矩阵。
 */

#include "Systems/NNCameraSystem.h"

#include <NNCore/Include/Math/GLM/ext/matrix_clip_space.hpp>

#include "Components/NNCameraComponent.h"
#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
void NNCameraSystem::Tick(NNRuntimeScene& scene, const float /*deltaTimeSeconds*/) noexcept
{
	scene.Query<NNCameraComponent>().Each(
		[](NNEntity /*handle*/, NNCameraComponent& camera)
		{
			switch (camera.Projection)
			{
			case NNProjectionType::Perspective:
				camera.ProjectionMatrix = glm::perspectiveRH_NO(
					glm::radians(camera.FovY),
					camera.AspectRatio,
					camera.NearPlane,
					camera.FarPlane);
				break;

			case NNProjectionType::Orthographic:
				camera.ProjectionMatrix = glm::orthoRH_NO(
					-camera.OrthoWidth  * 0.5f,
					 camera.OrthoWidth  * 0.5f,
					-camera.OrthoHeight * 0.5f,
					 camera.OrthoHeight * 0.5f,
					 camera.NearPlane,
					 camera.FarPlane);
				break;
			}
		});
}
} // namespace NN::Runtime::Scene
