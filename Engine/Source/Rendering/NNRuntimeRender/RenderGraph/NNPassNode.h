#pragma once

// ============================================================================
// NNPassNode — RenderGraph 中的渲染通道节点
// 描述一个渲染 Pass 及其资源依赖和执行回调
// ============================================================================

#include "NNResourceNode.h"

#include <cstdint>
#include <vector>
#include <functional>

namespace NN::Runtime::Render
{
    // 前向声明
    class INNCommandList;

    // 渲染通道节点 —— 包含输入输出资源声明和执行逻辑
    struct NNPassNode
    {
        uint32_t    Id          = 0;            // Pass 唯一 ID
        const char* Name        = nullptr;      // 调试名称

        // 资源依赖
        std::vector<uint32_t> InputResources;   // 本 Pass 读取的资源 ID 列表
        std::vector<uint32_t> OutputResources;  // 本 Pass 写入的资源 ID 列表

        // 渲染目标（可选 — 图形 Pass 使用）
        uint32_t    RenderTargetId = 0;         // 渲染目标资源 ID，0 表示无

        // 执行回调
        std::function<void(INNCommandList*)> Execute;

        // 拓扑排序辅助数据
        uint32_t    InDegree = 0;
        bool        Visited  = false;
    };
}
