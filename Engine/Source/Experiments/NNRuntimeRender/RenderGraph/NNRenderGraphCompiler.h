#pragma once

// ============================================================================
// NNRenderGraphCompiler — 渲染图编译器（预留扩展点）
// 当前阶段编译逻辑内置于 NNRenderGraphBuilder::Build()，
// 此头文件为未来扩展（资源屏障插入、Pass 合并等）预留接口
// ============================================================================

#include "NNRenderGraphBuilder.h"
#include "NNRenderGraph.h"

#include <cstdint>

namespace NN::Runtime::Render
{
    // 编译配置（预留）
    struct NNRenderGraphCompilerConfig
    {
        bool InsertBarriers     = true;     // 是否自动插入资源屏障
        bool ValidateGraph      = true;     // 是否校验图合法性
        bool OptimizePassOrder  = false;    // 是否优化 Pass 执行顺序
    };

    // 编译器类（当前直接委托给 Builder，未来可独立扩展）
    class NNRenderGraphCompiler
    {
    public:
        NNRenderGraphCompiler() = default;

        // 编译 Builder 中的图到 RenderGraph
        bool Compile(NNRenderGraphBuilder& builder,
                     NNRenderGraph& graph,
                     const NNRenderGraphCompilerConfig& config = {});

    private:
        NNRenderGraphCompilerConfig m_Config;
    };
}
