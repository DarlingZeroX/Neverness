// ============================================================================
// NNRenderGraphCompiler — 渲染图编译器实现
// 当前阶段直接委托给 NNRenderGraph，未来可扩展屏障插入和 Pass 合并
// ============================================================================

#include "NNRuntimeRender/RenderGraph/NNRenderGraphCompiler.h"

using namespace NN::Runtime::Render;

// ============================================================================
// Compile — 编译渲染图
// 当前实现直接委托给 NNRenderGraph::Compile()，
// config 参数为未来扩展预留（屏障插入、校验、Pass 合并等）
// ============================================================================
bool NNRenderGraphCompiler::Compile(NNRenderGraphBuilder& builder,
                                    NNRenderGraph& graph,
                                    const NNRenderGraphCompilerConfig& config)
{
    m_Config = config;

    // 当前阶段：直接编译，不做额外处理
    // 未来可在此插入：
    //   - 资源屏障自动插入
    //   - Pass 合并优化
    //   - 图合法性校验
    return graph.Compile(builder);
}
