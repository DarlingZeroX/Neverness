/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

#include "NNTextureCache.h"
#include <algorithm>

namespace NN::Runtime::Render
{

NNTextureCache::NNTextureCache(size_t maxMemoryBytes)
    : m_MaxMemory(maxMemoryBytes)
{
}

void NNTextureCache::Insert(uint64_t key, std::unique_ptr<NNTextureResource> resource, size_t memSize)
{
    // 如果已存在，先移除旧的
    auto it = m_Entries.find(key);
    if (it != m_Entries.end())
    {
        m_CurrentMemory -= it->second.MemorySize;
        m_LRUList.erase(it->second.LRUIt);
        m_Entries.erase(it);
    }

    // 插入到 LRU 头部（最近使用）
    m_LRUList.push_front(key);

    Entry entry;
    entry.Key = key;
    entry.Resource = std::move(resource);
    entry.MemorySize = memSize;
    entry.LRUIt = m_LRUList.begin();

    m_Entries.emplace(key, std::move(entry));
    m_CurrentMemory += memSize;
}

NNTextureResource* NNTextureCache::Get(uint64_t key)
{
    auto it = m_Entries.find(key);
    if (it == m_Entries.end())
        return nullptr;

    // 移动到 LRU 头部
    m_LRUList.erase(it->second.LRUIt);
    m_LRUList.push_front(key);
    it->second.LRUIt = m_LRUList.begin();

    return it->second.Resource.get();
}

bool NNTextureCache::Contains(uint64_t key) const
{
    return m_Entries.find(key) != m_Entries.end();
}

void NNTextureCache::Remove(uint64_t key)
{
    auto it = m_Entries.find(key);
    if (it == m_Entries.end())
        return;

    m_CurrentMemory -= it->second.MemorySize;
    m_LRUList.erase(it->second.LRUIt);
    m_Entries.erase(it);
}

size_t NNTextureCache::EvictToTarget(size_t targetBytes)
{
    size_t evicted = 0;

    while (m_CurrentMemory > targetBytes && !m_LRUList.empty())
    {
        uint64_t lruKey = m_LRUList.back();
        auto it = m_Entries.find(lruKey);
        if (it == m_Entries.end())
        {
            m_LRUList.pop_back();
            continue;
        }

        evicted += it->second.MemorySize;
        m_CurrentMemory -= it->second.MemorySize;
        m_LRUList.erase(it->second.LRUIt);
        m_Entries.erase(it);
    }

    return evicted;
}

} // namespace NN::Runtime::Render
