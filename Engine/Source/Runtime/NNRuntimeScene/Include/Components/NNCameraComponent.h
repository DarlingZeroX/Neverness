#pragma once

/**
 * @file NNCameraComponent.h
 * @brief 相机组件（POD，无虚表）：支持透视与正交两种投影。
 *
 * ProjectionMatrix 由 NNCameraSystem 在 Tick 中自动计算，
 * C# 端通过 [ComponentId] + blittable struct 镜像，只读取 ProjectionMatrix。
 */

#include <cstdint>
#include <NNCore/Interface/HVector.h>
#include "../../NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/**
	 * @brief 投影类型枚举。
	 */
	enum class NNProjectionType : std::uint32_t
	{
		Perspective  = 0,
		Orthographic = 1,
	};

	/**
	 * @brief 相机组件：纯数据，行为由 NNCameraSystem 驱动。
	 *
	 * 透视投影使用 FovY + AspectRatio；
	 * 正交投影使用 OrthoWidth + OrthoHeight。
	 * ProjectionMatrix 每帧由 NNCameraSystem 重算。
	 */
	struct NN_RUNTIME_SCENE_API NNCameraComponent
	{
		// ── 通用参数 ──
		NNProjectionType Projection = NNProjectionType::Perspective;
		float NearPlane   = 0.3f;
		float FarPlane    = 1000.0f;
		std::uint32_t _padding0 = 0u;

		// ── 透视参数 ──
		float FovY         = 45.0f;
		float AspectRatio  = 16.0f / 9.0f;

		// ── 正交参数 ──
		float OrthoWidth   = 10.0f;
		float OrthoHeight  = 10.0f;

		// ── 输出（由 NNCameraSystem 计算）──
		Core::matrix ProjectionMatrix{1.0f};

		NNCameraComponent() = default;
	};
} // namespace NN::Runtime::Scene
