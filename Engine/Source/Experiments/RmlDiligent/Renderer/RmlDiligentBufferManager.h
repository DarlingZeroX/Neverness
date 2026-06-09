#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"

#include <cstdint>
#include <vector>

namespace RmlDiligent {

/// 子分配结果：共享 buffer 中的偏移和大小，持有 chunk 引用以管理生命周期。
struct BufferAllocation {
    void* chunk = nullptr;          // 所属 BufferManager::Chunk*（用于引用计数）
    Diligent::IBuffer* buffer = nullptr;
    Diligent::Uint64 offset = 0;
    Diligent::Uint64 size = 0;
};

/// VB/IB 子分配池：从大 DYNAMIC buffer 线性子分配，避免每 geometry 创建独立 IMMUTABLE buffer。
///
/// 生命周期管理：
/// - chunk 创建时持久映射（MAP_FLAG_DISCARD），之后不 unmap
/// - AllocVertex/AllocIndex 增加 chunk 引用计数
/// - Release 时减少 chunk 引用计数（通过 BufferAllocation::chunk）
/// - 引用计数归零的 chunk 可被回收
/// - 不做每帧重置（RmlUi geometry 跨帧存在）
class BufferManager {
public:
    void Initialize(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context,
                    Diligent::Uint64 vbChunkSize, Diligent::Uint64 ibChunkSize);

    /// 分配 VB 子区域
    BufferAllocation AllocVertex(const void* data, Diligent::Uint64 dataSize);

    /// 分配 IB 子区域
    BufferAllocation AllocIndex(const void* data, Diligent::Uint64 dataSize);

    /// 释放分配（减少 chunk 引用计数）
    static void Release(BufferAllocation& alloc);

    /// 回收引用计数为 0 的 chunk
    void GarbageCollect();

    /// 每帧调用：执行 GC
    void BeginFrame();

private:
    struct Chunk {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
        Diligent::Uint8* mappedPtr = nullptr;
        Diligent::Uint64 capacity = 0;
        Diligent::Uint64 offset = 0;
        int refCount = 0;
    };

    Chunk& EnsureChunk(std::vector<Chunk>& chunks, Diligent::Uint64 dataSize,
                       Diligent::Uint64 chunkSize, Diligent::BIND_FLAGS bindFlags, const char* name);

    Diligent::IRenderDevice* m_Device = nullptr;
    Diligent::IDeviceContext* m_Context = nullptr;
    Diligent::Uint64 m_VBChunkSize = 0;
    Diligent::Uint64 m_IBChunkSize = 0;

    std::vector<Chunk> m_VBChunks;
    std::vector<Chunk> m_IBChunks;
};

} // namespace RmlDiligent
