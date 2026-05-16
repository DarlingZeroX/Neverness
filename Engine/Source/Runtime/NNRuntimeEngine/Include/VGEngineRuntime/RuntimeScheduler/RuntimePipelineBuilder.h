#pragma once

#include "NNRuntimeEngine/RuntimeScheduler/RuntimePipeline.h"

namespace visiongal::engine
{
/**
 * @brief **RuntimePipelineBuilder**（占位）：未来将提供 Fluent API 装配 **RuntimePipeline**（插入/移除阶段、条件 Tick）。
 *
 * 首包不实例化；避免首包过度设计，待 **P0-1+** 与编辑器/诊断需求明确后再落地。
 */
class RuntimePipelineBuilder final
{
public:
	[[nodiscard]] RuntimePipeline Build() const noexcept { return {}; }
};
} // namespace visiongal::engine
