#include "RmlDiligentBufferManager.h"

#include <cstring>
#include <iostream>

namespace RmlDiligent {

void BufferManager::Initialize(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context,
                               Diligent::Uint64 vbChunkSize, Diligent::Uint64 ibChunkSize)
{
    m_Device = device;
    m_Context = context;
    m_VBChunkSize = vbChunkSize;
    m_IBChunkSize = ibChunkSize;
}

BufferManager::Chunk& BufferManager::EnsureChunk(
    std::vector<Chunk>& chunks,
    Diligent::Uint64 dataSize,
    Diligent::Uint64 chunkSize,
    Diligent::BIND_FLAGS bindFlags,
    const char* name)
{
    const Diligent::Uint64 alignedSize = (dataSize + 15) & ~Diligent::Uint64(15);

    // 尝试使用最后一个 chunk（线性分配，不回溯）
    if (!chunks.empty()) {
        auto& chunk = chunks.back();
        if (chunk.offset + alignedSize <= chunk.capacity && chunk.mappedPtr) {
            return chunk;
        }
    }

    // 创建新 chunk
    const Diligent::Uint64 actualChunkSize = (alignedSize > chunkSize) ? alignedSize : chunkSize;

    Chunk newChunk;
    newChunk.capacity = actualChunkSize;
    newChunk.offset = 0;
    newChunk.refCount = 0;

    Diligent::BufferDesc desc;
    desc.Name = name;
    desc.Size = actualChunkSize;
    desc.BindFlags = bindFlags;
    desc.Usage = Diligent::USAGE_DYNAMIC;
    desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    m_Device->CreateBuffer(desc, nullptr, &newChunk.buffer);
    if (!newChunk.buffer) {
        std::cerr << "[FAIL] BufferManager: CreateBuffer failed (" << name << ", " << actualChunkSize << " bytes)" << std::endl;
        chunks.push_back(std::move(newChunk));
        return chunks.back();
    }

    // 持久映射（MAP_FLAG_DISCARD 只在初始映射时使用，之后不 unmap）
    void* ptr = nullptr;
    m_Context->MapBuffer(newChunk.buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, ptr);
    newChunk.mappedPtr = static_cast<Diligent::Uint8*>(ptr);
    if (!newChunk.mappedPtr) {
        std::cerr << "[FAIL] BufferManager: MapBuffer failed (" << name << ")" << std::endl;
    }

    std::cout << "[OK] BufferManager: new chunk (" << name << ", " << actualChunkSize << " bytes)" << std::endl;

    chunks.push_back(std::move(newChunk));
    return chunks.back();
}

BufferAllocation BufferManager::AllocVertex(const void* data, Diligent::Uint64 dataSize)
{
    auto& chunk = EnsureChunk(m_VBChunks, dataSize, m_VBChunkSize, Diligent::BIND_VERTEX_BUFFER, "BufferManager VB");
    if (!chunk.mappedPtr || !chunk.buffer) {
        return {};
    }

    const Diligent::Uint64 alignedSize = (dataSize + 15) & ~Diligent::Uint64(15);

    BufferAllocation alloc;
    alloc.chunk = static_cast<void*>(&chunk);
    alloc.buffer = chunk.buffer;
    alloc.offset = chunk.offset;
    alloc.size = dataSize;

    std::memcpy(chunk.mappedPtr + chunk.offset, data, static_cast<size_t>(dataSize));
    chunk.offset += alignedSize;
    ++chunk.refCount;

    return alloc;
}

BufferAllocation BufferManager::AllocIndex(const void* data, Diligent::Uint64 dataSize)
{
    auto& chunk = EnsureChunk(m_IBChunks, dataSize, m_IBChunkSize, Diligent::BIND_INDEX_BUFFER, "BufferManager IB");
    if (!chunk.mappedPtr || !chunk.buffer) {
        return {};
    }

    const Diligent::Uint64 alignedSize = (dataSize + 15) & ~Diligent::Uint64(15);

    BufferAllocation alloc;
    alloc.chunk = static_cast<void*>(&chunk);
    alloc.buffer = chunk.buffer;
    alloc.offset = chunk.offset;
    alloc.size = dataSize;

    std::memcpy(chunk.mappedPtr + chunk.offset, data, static_cast<size_t>(dataSize));
    chunk.offset += alignedSize;
    ++chunk.refCount;

    return alloc;
}

void BufferManager::Release(BufferAllocation& alloc)
{
    if (alloc.chunk) {
        auto* chunk = static_cast<Chunk*>(alloc.chunk);
        --chunk->refCount;
        alloc.chunk = nullptr;
        alloc.buffer = nullptr;
    }
}

void BufferManager::GarbageCollect()
{
    // 回收引用计数为 0 的 chunk（从末尾开始，保留至少 1 个 chunk）
    auto gc = [](std::vector<Chunk>& chunks) {
        // 从末尾开始移除空 chunk，但保留至少 1 个
        while (chunks.size() > 1 && chunks.back().refCount <= 0) {
            chunks.pop_back();
        }
    };
    gc(m_VBChunks);
    gc(m_IBChunks);
}

void BufferManager::BeginFrame()
{
    GarbageCollect();
}

} // namespace RmlDiligent
