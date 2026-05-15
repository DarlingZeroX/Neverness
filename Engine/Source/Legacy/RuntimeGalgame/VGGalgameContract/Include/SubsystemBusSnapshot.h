/*
 * SubsystemBusSnapshot — 窄快照句柄（Phase 7.8 / Phase 8 Contract）
 *
 * 仅承载调试 / 时间轴预留的不透明 token，不序列化具体子系统实现。
 * 默认 Snapshot/Restore 为空操作，避免总线成为上帝对象。
 *
 * 中文：Phase 8 起本类型属于 **VGGalgameContract**（纯 ABI 数据），与存档 / RuntimeState 解耦。
 */

#pragma once

#include <cstdint>

namespace VisionGal::GalGame
{
	struct SubsystemBusSnapshot
	{
		std::uint64_t opaqueToken = 0;
	};
}
