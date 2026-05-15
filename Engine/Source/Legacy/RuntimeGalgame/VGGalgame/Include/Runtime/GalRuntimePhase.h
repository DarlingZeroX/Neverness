/*
 * GalRuntimePhase — Gal 宿主运行时阶段枚举（Phase 8A / VGGalgame）
 *
 * 中文：供 **GalRuntimeCoordinator** 与各子系统在只读场景下判断「当前是否允许 Tick / 切场景 / 存档」，
 * 避免在 **Shutdown** 或 **Reset** 半途中重入脚本或 UI。后续可由 **IGalRuntimeSession** 暴露查询接口。
 */

#pragma once
#include <cstdint>
#include "../../VGGalgameConfig.h"

namespace VisionGal::GalGame
{
	/// 中文：与文档 Phase 8A-3 对齐；数值无持久化语义，禁止写入存档。
	enum class GalRuntimePhase : uint8_t
	{
		Uninitialized = 0,
		/// 中文：CreateSubsystem 批量构造子系统期间；此阶段禁止对外发布「已可玩」事件。
		Initializing,
		/// 中文：Initialize 完成且 RuntimeSession 已 Start，正常 Tick。
		Running,
		/// 中文：GalGameEngine::Reset 或等价全量清理执行中。
		Resetting,
		/// 中文：主场景切换回调内（Clear → 绑定新 Scene → 触发脚本延迟加载）。
		Transitioning,
		/// 中文：宿主析构或显式关闭链路上，按与 Initialize 相反顺序收尾。
		ShuttingDown,
	};
}
