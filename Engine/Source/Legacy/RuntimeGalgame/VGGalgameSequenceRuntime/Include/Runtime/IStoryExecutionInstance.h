/*
 * IStoryExecutionInstance — Galgame「Sequence 脚本」运行时执行实例契约（内核层）
 *
 * 与 VGGalgame 模块中具体类 `VisionGal::GalGame::StoryExecutionInstance` 区分：
 * 后者包装 IStoryScriptExecutor（宿主/Tick 入口），本接口表示 **VGGalgameSequenceRuntime 内部**
 * 的 Sequence Runtime Kernel 实例，负责单资产剪辑序列的状态机调度。
 *
 * Phase 2A：对外提供 Tick / Continue / GetState / QueryInterface，后续可扩展 Pause、调试接口等。
 */
#pragma once

#include "../SequenceRuntimeTypes.h"
#include <VGGalgameCore/Interface/IStoryScriptSystem.h>
#include <NNRuntimeCore/Interface/Interface.h>

namespace VisionGal::GalGame
{
	/**
	 * @brief Sequence 运行时执行实例抽象：由 SequenceExecutionInstance 实现。
	 */
	class VG_GSS_API IStorySequenceExecutionInstance: public IStoryExecutionInstance
	{
	public:
		~IStorySequenceExecutionInstance() override = default;

		[[nodiscard]] virtual ESSSequenceExecutorState GetState() const = 0;
	};
}
