/*
 * IEditorGalgameRuntimeBridge — 编辑器访问 Play/Preview 运行时的唯一入口（Phase 8.7）
 *
 * 中文：封装 `IGalRuntimeSession` / `ILuaRuntimeBridge` 等契约，避免编辑器代码散落 `GalGameEngineAccess`。
 * 具体实现可由宿主在注册阶段注入；未注入时接口返回 nullptr。
 */

#pragma once
#include "VGGalgameContract/Interface/IGalRuntimeSession.h"
#include "VGGalgameContract/Interface/ILuaRuntimeBridge.h"

namespace VisionGal::GalGame
{
	struct IEditorGalgameRuntimeBridge
	{
		virtual ~IEditorGalgameRuntimeBridge() = default;

		virtual IGalRuntimeSession* TryGetActivePlaySession() noexcept = 0;
		virtual ILuaRuntimeBridge* TryGetLuaBridge() noexcept = 0;
	};
}
