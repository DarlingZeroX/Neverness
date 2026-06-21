#pragma once

/**
 * @file NNCameraSystem.h
 * @brief 相机 System：每帧重算所有 NNCameraComponent 的 ProjectionMatrix。
 *
 * TickGroup 为 Update；根据 Projection 类型选择透视或正交矩阵。
 */

#include "../Systems/ISceneSystem.h"
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief Update 组内对 Camera 做批量访问，计算投影矩阵。
	 *
	 * 透视：glm::perspectiveRH_NO（右手坐标系，OpenGL NDC [-1,1]）。
	 * 正交：glm::orthoRH_NO。
	 */
	class NN_RUNTIME_SCENE_API NNCameraSystem final : public ISceneSystem
	{
	public:
		[[nodiscard]] NNSceneTickGroup TickGroup() const noexcept override
		{
			return NNSceneTickGroup::Update;
		}

		void Tick(NNRuntimeScene& scene, float deltaTimeSeconds) noexcept override;
	};
} // namespace NN::Runtime::Scene
