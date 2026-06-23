#pragma once

#include <cstdint>
#include "../../NNObject.h"

namespace NN::Runtime::Core
{
    // ========================================================================
    //  NNRenderHandle 鈥?64-bit 璧勬簮鍙ユ焺
    //
    //  缂栫爜: [绫诲瀷 16bit][搴忓彿 32bit][鐗堟湰 16bit]
    //  绫诲瀷: Texture=2, Buffer=3, Shader=4, Pipeline=5, Material=6, Mesh=7, RenderTarget=8
    //  鐗堟湰: 姣忔璧勬簮閲嶅缓閫掑锛岄槻姝?ABA 闂
    // ========================================================================

    using NNRenderHandle = uint64_t;
    constexpr NNRenderHandle NN_INVALID_HANDLE = 0;

    // HandleType: 鍙寘鍚?GPU 璧勬簮鍜岃祫浜э紝涓嶅寘鍚繍琛屾椂瀵硅薄
    // Device / SwapChain / CommandList 涓嶈繘 Registry锛岀敱 EngineContext 鐩存帴鎸佹湁
    enum class NNHandleType : uint16_t
    {
        Unknown      = 0,
        Texture      = 2,
        Buffer       = 3,
        Shader       = 4,
        Pipeline     = 5,
        Material     = 6,
        Mesh         = 7,
        RenderTarget = 8,
        Sampler      = 9,
    };

    inline NNRenderHandle MakeHandle(NNHandleType type, uint32_t index, uint16_t version)
    {
        return (uint64_t)(static_cast<uint16_t>(type)) << 48
             | (uint64_t)index << 16
             | version;
    }

    inline NNHandleType GetHandleType(NNRenderHandle h)
    {
        return static_cast<NNHandleType>((h >> 48) & 0xFFFF);
    }

    inline uint32_t GetHandleIndex(NNRenderHandle h)
    {
        return (h >> 16) & 0xFFFFFFFF;
    }

    inline uint16_t GetHandleVersion(NNRenderHandle h)
    {
        return h & 0xFFFF;
    }

    inline bool IsHandleValid(NNRenderHandle h)
    {
        return h != NN_INVALID_HANDLE && GetHandleType(h) != NNHandleType::Unknown;
    }

} // namespace NN::Runtime::Core
