/*
 * IRuntimeLayerGraph — 运行时图层图（Phase 8.5 Contract 骨架）
 *
 * 中文：替代「仅按 Scene Actor 挂载 Sprite/Audio」的隐式图层模型；为 Transition / Timeline / PostFX 预留统一遍历顺序。
 * 当前仅定义概念接口，具体实现见 VGGalgame `GalRuntimeLayerGraphAdapter`（由 LayeredSceneSystem 适配）。
 */

#pragma once
#include "../VGGalCoreConfig.h"
#include <cstdint>

namespace VisionGal::GalGame
{
	enum class GalRuntimeLayerKind : std::uint8_t
	{
		Background = 0,
		Character = 1,
		Effect = 2,
		UI = 3,
		Transition = 4,
	};

	struct IRuntimeLayerGraph
	{
		virtual ~IRuntimeLayerGraph() = default;

		/// 中文：按枚举顺序刷新/合成；具体语义由宿主实现演进。
		virtual void TickLayers(float deltaTime) = 0;
	};
}
