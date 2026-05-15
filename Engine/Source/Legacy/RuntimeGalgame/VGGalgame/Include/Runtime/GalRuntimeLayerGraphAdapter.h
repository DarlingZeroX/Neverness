/*
 * GalRuntimeLayerGraphAdapter — IRuntimeLayerGraph 的宿主适配（Phase 8.5）
 *
 * 中文：当前 TickLayers 为空实现；后续将 LayeredSceneSystem / TransitionManager 的合成顺序迁入此处，
 * 以替代分散在 GalGameEngine 与渲染管线中的隐式顺序。
 */

#pragma once

#include "VGGalgameContract/Interface/IRuntimeLayerGraph.h"
#include "../VGGalgameConfig.h"

namespace VisionGal::GalGame
{
	struct VG_GALGAME_API GalRuntimeLayerGraphAdapter final : public IRuntimeLayerGraph
	{
		void TickLayers(float deltaTime) override;
	};
}
