/*
 * SequenceRuntimeExecutionContext — RuntimeSystem 一次 Tick/Execute 可见的完整执行视图
 *
 * 在 Phase 2A 之前，RuntimeSystem 仅能访问 SSSequenceExecutionContext（引擎、资源表、序列数据）。
 * 本结构在保留 SharedContext 的同时，补充活动执行帧、内核实例指针、帧 Δt 以及对 **命令 API**
 * 的引用，禁止 RuntimeSystem 直接篡改 SequenceExecutionInstance 私有字段（跳转 / Wait / 帧栈
 * 一律经 SequenceRuntimeCommandAPI）。
 *
 * 生命周期：由 SequenceExecutionInstance::Tick 在栈上构造临时上下文；指针均非拥有，
 * 仅在单次 Tick/Execute 调用链内有效。
 */
#pragma once

#include "SequenceExecutionFrame.h"

#include "../SequenceExecutionContext.h"

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	class IStorySequenceExecutionInstance;
	class SequenceRuntimeCommandAPI;
}

namespace VisionGal
{
	/**
	 * @brief 扩展执行上下文：SharedContext + 活动帧 + 内核 + 命令门面。
	 */
	struct VG_GSS_API SequenceRuntimeExecutionContext
	{
		/// 原有的白盒数据：序列容器、资源管理器、IGalGameEngine*。
		SSSequenceExecutionContext* SharedContext = nullptr;

		/// 当前活动执行帧（Phase 2A 恒为帧栈栈顶）；可为 nullptr 表示未绑定内核。
		GalGame::SequenceExecutionFrame* ActiveFrame = nullptr;

		/// 拥有本上下文的执行实例（用于未来扩展 QueryKernel 等）。
		GalGame::IStorySequenceExecutionInstance* Instance = nullptr;

		/// 经内核授权的命令 API；RuntimeSystem 通过其发起 Continue / Jump 等。
		GalGame::SequenceRuntimeCommandAPI* CommandAPI = nullptr;

		/// 当前帧时间步长（秒），与 Tick(deltaTime) 入参一致。
		float DeltaTime = 0.f;
	};
}
