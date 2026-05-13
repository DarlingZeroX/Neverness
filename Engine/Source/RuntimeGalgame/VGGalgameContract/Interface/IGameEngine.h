/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzeroox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * 中文（Phase 8）：本接口迁入 **VGGalgameContract**；门面 API 已收缩为「总线 + 上下文 + 会话 + 多域 Runtime」。
 * **ABI 在 Phase 11 前仍为演进态**；与 SaveArchive / Lua 绑定联动变更时须同步文档与 schema 版本。
 */

#pragma once
#include "IGalGameContext.h"
#include "ISubsystemBus.h"
#include "IGalGameRuntime.h"
#include "VGCore/Interface/GameEngineInterface.h"

namespace VisionGal::GalGame
{
	struct IGalRuntimeSession;

	/**
	 * @brief GalGame 引擎接口（瘦门面）
	 *
	 * 中文：具体 ShowSprite / PlayAudio / Wait / Get*System 等能力请使用 **GetSubsystemBus()** 上各子系统；
	 * **GetRuntime()** 提供按域划分的聚合入口，便于 Headless / 测试替换实现。
	 */
	class IGalGameEngine : public ISubGameEngine
	{
	public:
		~IGalGameEngine() override = default;

		virtual void Reset() = 0;

		virtual ISubsystemBus* GetSubsystemBus() = 0;
		virtual IGalGameContext* GetContext() = 0;
		virtual IGalRuntimeSession* GetRuntimeSession() noexcept = 0;

		/// 中文：执行 / 存档 / 播放 / 变量 子域聚合；未装配子域时实现可返回 nullptr。
		virtual IGalGameRuntime* GetRuntime() noexcept = 0;
	};
}
