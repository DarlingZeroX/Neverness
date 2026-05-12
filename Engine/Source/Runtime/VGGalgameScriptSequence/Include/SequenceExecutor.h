/*
 * 历史头：原 SSSequenceExecutor 已演进为 SequenceExecutionInstance（Sequence Runtime Kernel）。
 *
 * 保留本文件与类型别名，避免外部 include 路径大规模变更；新代码请优先包含
 * `Runtime/SequenceExecutionInstance.h` 与 `Runtime/IStoryExecutionInstance.h`。
 */
#pragma once

#include "Runtime/SequenceExecutionInstance.h"

namespace VisionGal::GalGame
{
	/// 与旧 SSSequenceExecutor 二进制兼容命名：实为 SequenceExecutionInstance。
	using SSSequenceExecutor = SequenceExecutionInstance;
}
