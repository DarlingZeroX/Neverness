/*
 * ILuaRuntimeBridge — Lua 与 Gal 运行时之间的受控桥梁（Phase 8 Contract 骨架）
 *
 * 中文：目标为禁止 Lua 绑定层直接持有 IGalGameEngine 并「穿透」访问宿主实现；
 * 后续将 GalGameLuaBinding 的 GetEngine 迁移为 GetLuaBridge()，仅暴露会话/总线/白名单 API。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IGalRuntimeSession;
	struct ISubsystemBus;

	struct VG_GALGAME_CORE_API ILuaRuntimeBridge
	{
		virtual ~ILuaRuntimeBridge() = default;

		/// 中文：当前线程活跃的 Gal 运行时会话（可能为空）。
		virtual IGalRuntimeSession* GetActiveSession() noexcept = 0;

		/// 中文：等价于会话上的子系统总线；不得绕过会话直接 new 子系统。
		virtual ISubsystemBus* GetSubsystemBus() noexcept = 0;
	};
}
