#pragma once

// ============================================================================
// NNResourceNode — RenderGraph 中的资源节点定义
// 描述一个纹理或缓冲区资源在渲染图中的元数据
// ============================================================================

#include <cstdint>
#include <limits>

namespace NN::Runtime::Render
{
    // 资源类型：纹理 或 缓冲区
    enum class NNResourceType : uint8_t
    {
        Texture,
        Buffer
    };

    // 资源访问模式：读、写、读写
    enum class NNResourceAccess : uint8_t
    {
        Read,
        Write,
        ReadWrite
    };

    // 资源节点 —— 记录单个资源在渲染图中的属性和生命周期
    struct NNResourceNode
    {
        uint32_t        Id          = 0;                // 资源唯一 ID
        NNResourceType  Type        = NNResourceType::Texture;
        NNResourceAccess Access     = NNResourceAccess::Read;
        const char*     Name        = nullptr;           // 调试名称

        // 纹理参数
        uint32_t        Width       = 0;
        uint32_t        Height      = 0;
        uint32_t        Format      = 0;                 // NNPixelFormat 枚举值

        // 缓冲区参数
        uint32_t        Size        = 0;

        // 生命周期（由 Compiler 自动计算）
        uint32_t        FirstUsePass = (std::numeric_limits<uint32_t>::max)();
        uint32_t        LastUsePass  = 0;
    };
}
