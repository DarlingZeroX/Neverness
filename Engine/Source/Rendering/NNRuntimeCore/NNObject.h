#pragma once

#include <cstdint>
#include <atomic>
#include <cassert>

namespace NN::Runtime::Core
{
    // ========================================================================
    //  INNObject 鈥?鎵€鏈?Runtime 瀵硅薄缁熶竴鍩虹被
    //  鎵€鏈夋覆鏌撳璞★紙INNTexture, INNBuffer, INNShader, ...锛夐兘缁ф壙姝ょ被
    // ========================================================================

    class INNObject
    {
    public:
        virtual ~INNObject() = default;

        virtual uint32_t AddRef() = 0;
        virtual uint32_t Release() = 0;
        virtual uint32_t GetRefCount() const = 0;
    };

    // ========================================================================
    //  NNRef<T> 鈥?鏅鸿兘鎸囬拡
    // ========================================================================

    template<typename T>
    class NNRef
    {
    public:
        NNRef() : m_Ptr(nullptr) {}

        NNRef(std::nullptr_t) : m_Ptr(nullptr) {}

        explicit NNRef(T* ptr) : m_Ptr(ptr)
        {
            if (m_Ptr) m_Ptr->AddRef();
        }

        NNRef(const NNRef& other) : m_Ptr(other.m_Ptr)
        {
            if (m_Ptr) m_Ptr->AddRef();
        }

        NNRef(NNRef&& other) noexcept : m_Ptr(other.m_Ptr)
        {
            other.m_Ptr = nullptr;
        }

        template<typename U>
        NNRef(const NNRef<U>& other) : m_Ptr(static_cast<T*>(other.Get()))
        {
            if (m_Ptr) m_Ptr->AddRef();
        }

        template<typename U>
        NNRef(NNRef<U>&& other) noexcept : m_Ptr(static_cast<T*>(other.Get()))
        {
            other.m_Ptr = nullptr;
        }

        ~NNRef()
        {
            if (m_Ptr) m_Ptr->Release();
        }

        NNRef& operator=(const NNRef& other)
        {
            if (this != &other)
            {
                if (m_Ptr) m_Ptr->Release();
                m_Ptr = other.m_Ptr;
                if (m_Ptr) m_Ptr->AddRef();
            }
            return *this;
        }

        NNRef& operator=(NNRef&& other) noexcept
        {
            if (this != &other)
            {
                if (m_Ptr) m_Ptr->Release();
                m_Ptr = other.m_Ptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

        NNRef& operator=(std::nullptr_t)
        {
            if (m_Ptr) m_Ptr->Release();
            m_Ptr = nullptr;
            return *this;
        }

        T* operator->() const { assert(m_Ptr); return m_Ptr; }
        T& operator*() const { assert(m_Ptr); return *m_Ptr; }
        T* Get() const { return m_Ptr; }

        explicit operator bool() const { return m_Ptr != nullptr; }

        bool operator==(const NNRef& other) const { return m_Ptr == other.m_Ptr; }
        bool operator!=(const NNRef& other) const { return m_Ptr != other.m_Ptr; }
        bool operator==(std::nullptr_t) const { return m_Ptr == nullptr; }
        bool operator!=(std::nullptr_t) const { return m_Ptr != nullptr; }

        // Release and return raw pointer (caller takes ownership)
        T* Detach()
        {
            T* ptr = m_Ptr;
            m_Ptr = nullptr;
            return ptr;
        }

        // Reset with new pointer
        void Reset(T* ptr = nullptr)
        {
            if (m_Ptr) m_Ptr->Release();
            m_Ptr = ptr;
            if (m_Ptr) m_Ptr->AddRef();
        }

    private:
        T* m_Ptr;

        template<typename U> friend class NNRef;
    };

    // ========================================================================
    //  Helper
    // ========================================================================

    template<typename T, typename... Args>
    NNRef<T> MakeNNRef(Args&&... args)
    {
        return NNRef<T>(new T(std::forward<Args>(args)...));
    }

} // namespace NN::Runtime::Core
