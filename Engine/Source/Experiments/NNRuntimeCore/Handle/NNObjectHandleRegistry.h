#pragma once

#include "NNHandleTypes.h"
#include <unordered_map>
#include <mutex>
#include <vector>

namespace NN::Runtime::Core
{
    class NNObjectHandleRegistry
    {
    public:
        NNObjectHandleRegistry() = default;
        ~NNObjectHandleRegistry() = default;

        NNObjectHandleRegistry(const NNObjectHandleRegistry&) = delete;
        NNObjectHandleRegistry& operator=(const NNObjectHandleRegistry&) = delete;

        NNRenderHandle Register(NNHandleType type, NNRef<INNObject> resource)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            uint32_t& nextIdx = m_NextIndex[type];
            uint32_t index = nextIdx++;
            uint16_t ver = 1;
            m_Entries[index] = Entry{ type, ver, std::move(resource) };
            return MakeHandle(type, index, ver);
        }

        INNObject* Get(NNRenderHandle handle)
        {
            if (!IsHandleValid(handle)) return nullptr;
            std::lock_guard<std::mutex> lock(m_Mutex);
            uint32_t index = GetHandleIndex(handle);
            uint16_t ver = GetHandleVersion(handle);
            auto it = m_Entries.find(index);
            if (it == m_Entries.end()) return nullptr;
            if (it->second.version != ver) return nullptr;
            return it->second.resource.Get();
        }

        template<typename T>
        T* GetAs(NNRenderHandle handle)
        {
            return static_cast<T*>(Get(handle));
        }

        void Release(NNRenderHandle handle)
        {
            if (!IsHandleValid(handle)) return;
            std::lock_guard<std::mutex> lock(m_Mutex);
            uint32_t index = GetHandleIndex(handle);
            uint16_t ver = GetHandleVersion(handle);
            auto it = m_Entries.find(index);
            if (it == m_Entries.end()) return;
            if (it->second.version != ver) return;
            m_Entries.erase(it);
        }

        void Replace(NNRenderHandle handle, NNRef<INNObject> newResource)
        {
            if (!IsHandleValid(handle)) return;
            std::lock_guard<std::mutex> lock(m_Mutex);
            uint32_t index = GetHandleIndex(handle);
            uint16_t ver = GetHandleVersion(handle);
            auto it = m_Entries.find(index);
            if (it == m_Entries.end()) return;
            if (it->second.version != ver) return;
            it->second.resource = std::move(newResource);
        }

        size_t GetCount() const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Entries.size();
        }

        size_t GetCount(NNHandleType type) const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            size_t count = 0;
            for (const auto& [idx, entry] : m_Entries)
            {
                if (entry.type == type) count++;
            }
            return count;
        }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Entries.clear();
            m_NextIndex.clear();
        }

    private:
        struct Entry
        {
            NNHandleType type;
            uint16_t version;
            NNRef<INNObject> resource;
        };

        std::unordered_map<uint32_t, Entry> m_Entries;
        std::unordered_map<NNHandleType, uint32_t> m_NextIndex;
        mutable std::mutex m_Mutex;
    };

} // namespace NN::Runtime::Core
