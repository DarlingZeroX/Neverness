/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzeroox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "GalGameRuntimeState.h"
#include "GalGameEvent.h"
#include "ArchiveDataContainer.h"
#include "VGGalgameContract/Interface/IGalGameContext.h"
#include "VGGalgameContract/Interface/ISubsystemBus.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief Gal 运行时上下文：纯数据 + 事件总线（Phase 8）。
	 *
	 * 中文：**不再持有 IGalGameEngine***，避免 Context 成为「半服务定位器」；
	 * 引擎 / 总线 / 会话由执行栈或宿主显式注入（见 IGalRuntimeSession、ISubsystemBus）。
	 */
	struct GalGameContext : public IGalGameContext
	{
		GalGameContext()
		{
			archiveData = MakeRef<ArchiveDataContainer>();
		}

		~GalGameContext() override = default;

		GalEngineEventBus engineEventBus;
		GalGameUIEventBus uiEventBus;

		GalGameRuntimeState runtimeState;
		Ref<ArchiveDataContainer> archiveData = nullptr;

		/**
		 * @brief 推荐的唯一构造入口：集中初始化 `archiveData`。
		 * @param bus 可选；预留与宿主总线一致性校验（当前未强校验）。
		 */
		static Ref<GalGameContext> Create(ISubsystemBus* bus = nullptr)
		{
			Ref<GalGameContext> ctx = MakeRef<GalGameContext>();
			(void)bus;
			return ctx;
		}
	};
}
