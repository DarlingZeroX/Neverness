/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include "NNTextureResource.h"
#include <list>
#include <unordered_map>
#include <memory>

namespace NN::Runtime::Render
{

/// LRU 纹理缓存
/// 管理 GPU 资源的驻留/驱逐
class NNTextureCache
{
public:
    explicit NNTextureCache(size_t maxMemoryBytes = 256ull * 1024 * 1024);
    ~NNTextureCache() = default;

    // 禁止拷贝和移动
    NNTextureCache(const NNTextureCache&) = delete;
    NNTextureCache& operator=(const NNTextureCache&) = delete;

    void Insert(uint64_t key, std::unique_ptr<NNTextureResource> resource, size_t memSize);
    NNTextureResource* Get(uint64_t key);
    bool Contains(uint64_t key) const;
    void Remove(uint64_t key);

    /// 驱逐到目标内存以下，返回驱逐的字节数
    size_t EvictToTarget(size_t targetBytes);

    size_t GetCurrentMemory() const { return m_CurrentMemory; }
    size_t GetMaxMemory() const { return m_MaxMemory; }
    size_t GetCount() const { return m_Entries.size(); }

    void SetMaxMemory(size_t maxBytes) { m_MaxMemory = maxBytes; }

private:
    struct Entry
    {
        uint64_t Key;
        std::unique_ptr<NNTextureResource> Resource;
        size_t MemorySize;
        std::list<uint64_t>::iterator LRUIt;
    };

    std::unordered_map<uint64_t, Entry> m_Entries;
    std::list<uint64_t> m_LRUList;  // front = most recent, back = least recent
    size_t m_CurrentMemory = 0;
    size_t m_MaxMemory;
};

} // namespace NN::Runtime::Render
