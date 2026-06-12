// ============================================================================
// NNRenderGraph — 渲染图实现
// 编译 Builder 数据并按拓扑顺序执行所有 Pass
// ============================================================================

#include "NNRuntimeRender/RenderGraph/NNRenderGraph.h"
#include "NNRuntimeRender/Command/INNCommandList.h"

using namespace NN::Runtime::Render;

// ============================================================================
// Compile — 从 Builder 编译渲染图
// 调用 Builder::Build() 执行拓扑排序，然后拷贝结果
// ============================================================================
bool NNRenderGraph::Compile(NNRenderGraphBuilder& builder)
{
    if (!builder.Build())
        return false;

    m_Resources      = builder.GetResources();
    m_Passes         = builder.GetPasses();
    m_ExecutionOrder = builder.GetExecutionOrder();
    return true;
}

// ============================================================================
// Execute — 按拓扑顺序依次执行所有 Pass
// ============================================================================
bool NNRenderGraph::Execute(INNCommandList* cmdList)
{
    if (!cmdList)
        return false;

    for (uint32_t passId : m_ExecutionOrder)
    {
        auto& pass = m_Passes[passId];
        if (pass.Execute)
            pass.Execute(cmdList);
    }
    return true;
}
