/*
 * IRuntimeDebugBridge — 编辑器 / Inspector 与运行时解耦（Phase 8 占位）
 *
 * 中文：后续由 VGGalgameEditorRuntime 实现；断点、单步、变量快照均经本接口，禁止编辑器直接 include DialogueSystem 等实现头。
 */

#pragma once
#include "../VGGalCoreConfig.h"

namespace VisionGal::GalGame
{
	struct IRuntimeDebugBridge
	{
		virtual ~IRuntimeDebugBridge() = default;
	};
}
