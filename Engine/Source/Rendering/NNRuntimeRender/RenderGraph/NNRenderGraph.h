#pragma once

// ============================================================================
// NNRenderGraph — 渲染图
// 持有编译后的渲染图数据，支持按拓扑顺序执行所有 Pass
// ============================================================================

#include "NNRenderGraphBuilder.h"

#include <cstdint>
#include <vector>

namespace NN::Runtime::Render
{
    // 前向声明
    class INNCommandList;
    class INNRenderDevice;

    class NNRenderGraph
    {
    public:
        NNRenderGraph()  = default;
        ~NNRenderGraph() = default;

        // 从 Builder 编译渲染图（执行拓扑排序 + 生命周期计算）
        bool Compile(NNRenderGraphBuilder& builder);

        // 按拓扑顺序执行所有 Pass
        bool Execute(INNCommandList* cmdList);

        // 获取执行顺序（调试用）
        const std::vector<uint32_t>& GetExecutionOrder() const { return m_ExecutionOrder; }

    private:
        std::vector<NNResourceNode> m_Resources;
        std::vector<NNPassNode>     m_Passes;
        std::vector<uint32_t>       m_ExecutionOrder;
    };
}
