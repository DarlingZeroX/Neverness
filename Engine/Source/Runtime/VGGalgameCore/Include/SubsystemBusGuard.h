/*
 * SubsystemBusGuard — ISubsystemBus 空指针安全包装（Phase 7）
 *
 * 用于 Tick / Continue / 序列执行等路径：无总线时不应调用子系统，避免隐式依赖全局引擎。
 */

#pragma once

#include "VGGalgameCore/Interface/ISubsystemBus.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 轻量 RAII 式总线句柄；不拥有 bus 指向的对象。
	 *
	 * 典型用法：`SubsystemBusGuard g(bus); if (!g) return; g.Get()->Dialogue()->...`
	 */
	struct SubsystemBusGuard
	{
		ISubsystemBus* Bus = nullptr;

		explicit SubsystemBusGuard(ISubsystemBus* bus) noexcept
			: Bus(bus)
		{
		}

		[[nodiscard]] explicit operator bool() const noexcept { return Bus != nullptr; }

		[[nodiscard]] ISubsystemBus* Get() const noexcept { return Bus; }
	};
}
