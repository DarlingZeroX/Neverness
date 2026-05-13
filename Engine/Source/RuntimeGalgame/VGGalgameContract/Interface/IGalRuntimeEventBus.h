/*
 * IGalRuntimeEventBus — 统一运行时事件总线（Phase 8 骨架）
 *
 * 中文：目标为合并 / 包装 GalEngineEventBus 与 GalGameUIEventBus，支持优先级、延迟派发、sticky；
 * 当前仅占位，避免各模块继续散落直连两套总线。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IGalRuntimeEventBus
	{
		virtual ~IGalRuntimeEventBus() = default;
	};
}
