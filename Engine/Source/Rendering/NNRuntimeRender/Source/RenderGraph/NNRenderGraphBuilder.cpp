// ============================================================================
// NNRenderGraphBuilder — 渲染图构建器实现
// 包含拓扑排序（Kahn 算法）、资源生命周期计算和 DOT 导出
// ============================================================================

#include "NNRuntimeRender/RenderGraph/NNRenderGraphBuilder.h"
#include "NNRuntimeRender/Command/INNCommandList.h"

#include <queue>
#include <fstream>
#include <cassert>

using namespace NN::Runtime::Render;

// ============================================================================
// AddResource — 注册一个新资源节点，返回其 ID
// ============================================================================
uint32_t NNRenderGraphBuilder::AddResource(const char* name,
                                           NNResourceType type,
                                           uint32_t width,
                                           uint32_t height,
                                           uint32_t format)
{
    NNResourceNode node;
    node.Id     = static_cast<uint32_t>(m_Resources.size());
    node.Type   = type;
    node.Name   = name;
    node.Width  = width;
    node.Height = height;
    node.Format = format;
    m_Resources.push_back(node);
    return node.Id;
}

// ============================================================================
// AddPass — 注册一个新渲染通道，返回其 ID
// ============================================================================
uint32_t NNRenderGraphBuilder::AddPass(const char* name,
                                       std::function<void(INNCommandList*)> execute)
{
    NNPassNode node;
    node.Id      = static_cast<uint32_t>(m_Passes.size());
    node.Name    = name;
    node.Execute = std::move(execute);
    m_Passes.push_back(node);
    return node.Id;
}

// ============================================================================
// DeclareInput — 声明 passId 读取 resourceId
// ============================================================================
void NNRenderGraphBuilder::DeclareInput(uint32_t passId, uint32_t resourceId)
{
    assert(passId < m_Passes.size());
    m_Passes[passId].InputResources.push_back(resourceId);
}

// ============================================================================
// DeclareOutput — 声明 passId 写入 resourceId
// ============================================================================
void NNRenderGraphBuilder::DeclareOutput(uint32_t passId, uint32_t resourceId)
{
    assert(passId < m_Passes.size());
    m_Passes[passId].OutputResources.push_back(resourceId);
}

// ============================================================================
// SetRenderTarget — 设置 passId 的渲染目标
// ============================================================================
void NNRenderGraphBuilder::SetRenderTarget(uint32_t passId, uint32_t resourceId)
{
    assert(passId < m_Passes.size());
    m_Passes[passId].RenderTargetId = resourceId;
}

// ============================================================================
// Build — 执行拓扑排序和资源生命周期计算
// ============================================================================
bool NNRenderGraphBuilder::Build()
{
    if (m_Passes.empty())
        return true;

    if (!TopologicalSort())
        return false;

    ComputeResourceLifetime();
    return true;
}

// ============================================================================
// TopologicalSort — Kahn 算法
// 计算每个 Pass 的入度，然后从入度为 0 的节点开始 BFS
// ============================================================================
bool NNRenderGraphBuilder::TopologicalSort()
{
    const uint32_t passCount = static_cast<uint32_t>(m_Passes.size());

    // 重置入度
    for (auto& pass : m_Passes)
        pass.InDegree = 0;

    // 计算入度：如果 Pass B 读取了资源 R，而 Pass A 写入了资源 R，则 B 依赖 A
    for (auto& pass : m_Passes)
    {
        for (uint32_t inputRes : pass.InputResources)
        {
            for (auto& other : m_Passes)
            {
                if (other.Id == pass.Id)
                    continue;

                for (uint32_t outputRes : other.OutputResources)
                {
                    if (outputRes == inputRes)
                    {
                        pass.InDegree++;
                    }
                }
            }
        }
    }

    // Kahn 算法 BFS
    std::queue<uint32_t> readyQueue;
    for (auto& pass : m_Passes)
    {
        if (pass.InDegree == 0)
            readyQueue.push(pass.Id);
    }

    m_ExecutionOrder.clear();
    m_ExecutionOrder.reserve(passCount);

    while (!readyQueue.empty())
    {
        uint32_t id = readyQueue.front();
        readyQueue.pop();
        m_ExecutionOrder.push_back(id);

        // 找到依赖当前 Pass 输出的其他 Pass，减少其入度
        for (auto& pass : m_Passes)
        {
            if (pass.Id == id)
                continue;

            for (uint32_t inputRes : pass.InputResources)
            {
                for (uint32_t outputRes : m_Passes[id].OutputResources)
                {
                    if (outputRes == inputRes)
                    {
                        if (pass.InDegree > 0)
                            pass.InDegree--;

                        if (pass.InDegree == 0)
                            readyQueue.push(pass.Id);
                    }
                }
            }
        }
    }

    // 如果排序结果数量 != Pass 总数，说明存在环
    return m_ExecutionOrder.size() == passCount;
}

// ============================================================================
// ComputeResourceLifetime — 遍历执行顺序，记录每个资源的首次和末次使用 Pass
// ============================================================================
void NNRenderGraphBuilder::ComputeResourceLifetime()
{
    // 重置生命周期
    for (auto& res : m_Resources)
    {
        res.FirstUsePass = (std::numeric_limits<uint32_t>::max)();
        res.LastUsePass  = 0;
    }

    // 按执行顺序遍历
    for (uint32_t execIndex = 0; execIndex < m_ExecutionOrder.size(); ++execIndex)
    {
        uint32_t passId = m_ExecutionOrder[execIndex];
        const auto& pass = m_Passes[passId];

        auto updateLifetime = [&](uint32_t resId)
        {
            if (resId < m_Resources.size())
            {
                if (execIndex < m_Resources[resId].FirstUsePass)
                    m_Resources[resId].FirstUsePass = execIndex;
                if (execIndex > m_Resources[resId].LastUsePass)
                    m_Resources[resId].LastUsePass = execIndex;
            }
        };

        for (uint32_t resId : pass.InputResources)
            updateLifetime(resId);

        for (uint32_t resId : pass.OutputResources)
            updateLifetime(resId);
    }
}

// ============================================================================
// ExportDOT — 导出 Graphviz DOT 格式文件
// 节点 = Pass，边 = 资源依赖
// ============================================================================
void NNRenderGraphBuilder::ExportDOT(const char* filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
        return;

    file << "digraph RenderGraph {\n";
    file << "    rankdir=LR;\n";
    file << "    node [shape=box, style=filled, fillcolor=lightblue];\n\n";

    // Pass 节点
    for (const auto& pass : m_Passes)
    {
        file << "    P" << pass.Id << " [label=\"" << (pass.Name ? pass.Name : "?") << "\"];\n";
    }
    file << "\n";

    // 资源节点
    file << "    node [shape=ellipse, style=filled, fillcolor=lightyellow];\n\n";
    for (const auto& res : m_Resources)
    {
        file << "    R" << res.Id << " [label=\"" << (res.Name ? res.Name : "?") << "\"];\n";
    }
    file << "\n";

    // 输入边（资源 -> Pass）
    for (const auto& pass : m_Passes)
    {
        for (uint32_t resId : pass.InputResources)
        {
            file << "    R" << resId << " -> P" << pass.Id << " [color=blue];\n";
        }
    }

    // 输出边（Pass -> 资源）
    for (const auto& pass : m_Passes)
    {
        for (uint32_t resId : pass.OutputResources)
        {
            file << "    P" << pass.Id << " -> R" << resId << " [color=red];\n";
        }
    }

    file << "}\n";
    file.close();
}
