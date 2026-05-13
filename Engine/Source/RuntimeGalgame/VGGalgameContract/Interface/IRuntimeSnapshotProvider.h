/*
 * IRuntimeSnapshotProvider — 存档树中的运行时快照片段（Phase 8.6 Contract 骨架）
 *
 * 中文：SaveArchive 未来将聚合各子系统 `CaptureSnapshot` / `RestoreSnapshot`；
 * 本接口由各 Runtime（对白 / 序列 / UI / 场景）实现并注册到快照中心（后续落地）。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IRuntimeSnapshotProvider
	{
		virtual ~IRuntimeSnapshotProvider() = default;

		/// 中文：稳定段键，例如 "DialogueRuntime" / "SequenceRuntime"。
		virtual const char* SnapshotSectionId() const noexcept = 0;
	};
}
