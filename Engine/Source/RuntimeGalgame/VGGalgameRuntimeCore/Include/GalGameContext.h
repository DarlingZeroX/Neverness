/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
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
	class IGalGameEngine;

	/**
	 * @brief Gal 运行时上下文：纯数据 + 事件总线（Phase 7 数据层）。
	 *
	 * **Engine**：弱引用语义，宿主 `GalGameEngine` 拥有生命周期；`Reset` / 析构时必须置空，禁止悬空。
	 */
	struct GalGameContext : public IGalGameContext
	{
		GalGameContext()
		{
			archiveData = MakeRef<ArchiveDataContainer>();
		}

		~GalGameContext() override = default;

		IGalGameEngine* Engine = nullptr;

		GalEngineEventBus engineEventBus;
		GalGameUIEventBus uiEventBus;

		GalGameRuntimeState runtimeState;
		Ref<ArchiveDataContainer> archiveData = nullptr;

		/**
		 * @brief 推荐的唯一构造入口：集中初始化 `archiveData` 与引擎弱引用。
		 * @param bus 可选；预留与宿主总线一致性校验（当前未强校验）。
		 */
		static Ref<GalGameContext> Create(IGalGameEngine* engine, ISubsystemBus* bus = nullptr)
		{
			Ref<GalGameContext> ctx = MakeRef<GalGameContext>();
			ctx->Engine = engine;
			(void)bus;
			return ctx;
		}
	};
}
