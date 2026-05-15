/*
 * IGalRuntimeEventBus — 统一运行时事件总线（Phase 8 骨架）
 *
 * 中文：目标为合并 / 包装 **GalEngineEventBus** 与 **GalGameUIEventBus**，支持优先级、延迟派发、sticky；
 * 当前仅占位。宿主级生命周期事件已先行落在 **GalEngineEventBus::OnRuntimeLifecycleEvent**（见 **GalGameEvent.h**），
 * 未来可将该委托迁移为本接口实现，以避免双总线长期并存。
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
