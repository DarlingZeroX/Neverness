#pragma once

// ============================================================================
// NNRenderGraphBuilder — 渲染图构建器
// 提供 Fluent API 用于声明资源、Pass 和依赖关系，
// 最终通过 Build() 生成拓扑排序的执行序列
// ============================================================================

#include "NNResourceNode.h"
#include "NNPassNode.h"

#include <cstdint>
#include <vector>

namespace NN::Runtime::Render
{
    // 前向声明
    class INNCommandList;

    class NNRenderGraphBuilder
    {
    public:
        NNRenderGraphBuilder() = default;

        // ---- 资源声明 ----

        // 添加一个资源到图中，返回资源 ID
        uint32_t AddResource(const char* name,
                             NNResourceType type,
                             uint32_t width  = 0,
                             uint32_t height = 0,
                             uint32_t format = 0);

        // ---- Pass 声明 ----

        // 添加一个渲染通道，返回 Pass ID
        uint32_t AddPass(const char* name,
                         std::function<void(INNCommandList*)> execute);

        // 声明 Pass 读取某个资源
        void DeclareInput(uint32_t passId, uint32_t resourceId);

        // 声明 Pass 写入某个资源
        void DeclareOutput(uint32_t passId, uint32_t resourceId);

        // 设置 Pass 的渲染目标
        void SetRenderTarget(uint32_t passId, uint32_t resourceId);

        // ---- 构建 ----

        // 执行拓扑排序和生命周期计算，返回 false 表示存在环
        bool Build();

        // ---- 访问构建结果 ----

        const std::vector<NNResourceNode>& GetResources()     const { return m_Resources; }
        const std::vector<NNPassNode>&     GetPasses()         const { return m_Passes; }
        const std::vector<uint32_t>&       GetExecutionOrder() const { return m_ExecutionOrder; }

        // ---- 调试 ----

        // 导出 DOT 格式文件，可用 Graphviz 可视化
        void ExportDOT(const char* filename) const;

    private:
        std::vector<NNResourceNode> m_Resources;
        std::vector<NNPassNode>     m_Passes;
        std::vector<uint32_t>       m_ExecutionOrder;   // 拓扑排序后的 Pass ID 序列

        // Kahn 算法拓扑排序
        bool TopologicalSort();

        // 计算每个资源的 FirstUsePass / LastUsePass
        void ComputeResourceLifetime();
    };
}
