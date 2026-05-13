/*
 * GalGameLayoutUtils — 设计分辨率与精灵对齐偏移（与引擎单例解耦）
 *
 * 原位于 GameEngineCore；SubsystemBus 重构后仅保留布局数学，供立绘 / 精灵对齐使用。
 */

#pragma once
#include "../VGGRCExport.h"
#include "VGGalgameContract/VGGalCoreConfig.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	struct VG_RUNTIME_GALCORE_API GalGameLayoutUtils
	{
		static void SetDesignSize(float2 size);
		static float2 GetDesignSize();
		static float GetSpriteYOffset(float size_y);
		static float GetSpriteXOffset(float size_x);
	};
}
