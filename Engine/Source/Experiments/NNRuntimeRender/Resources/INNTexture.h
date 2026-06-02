#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Render
{
    // using namespace removed - use fully qualified names

    // ========================================================================
    //  Texture 璧勬簮鎺ュ彛
    // ========================================================================

    enum class NNTextureDimension : uint8_t
    {
        Tex1D,
        Tex2D,
        Tex3D,
        TexCube
    };

    enum class NNTextureUsage : uint8_t
    {
        Default,
        RenderTarget,
        DepthStencil,
        Staging
    };

    enum class NNPixelFormat : uint8_t
    {
        Unknown = 0,
        RGBA8_UNORM,
        RGBA8_SRGB,
        BGRA8_UNORM,
        R32_FLOAT,
        RG32_FLOAT,
        RGBA32_FLOAT,
        D32_FLOAT,
        D24_UNORM_S8_UINT,
        BC1_UNORM,
        BC3_UNORM,
        BC5_UNORM,
        BC7_UNORM,
    };

    struct NNTextureDesc
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t Depth = 1;
        uint32_t MipLevels = 1;
        uint32_t ArraySize = 1;
        NNTextureDimension Dimension = NNTextureDimension::Tex2D;
        NNTextureUsage Usage = NNTextureUsage::Default;
        NNPixelFormat Format = NNPixelFormat::RGBA8_UNORM;
        uint32_t SampleCount = 1;
        const char* DebugName = nullptr;
    };

    class INNTexture : public NN::Runtime::Core::INNObject
    {
    public:
        virtual const NNTextureDesc& GetDesc() const = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        // 涓嶆毚闇插簳灞傜被鍨嬶紙GetNativeHandle 绛夛級
        // 鍚庣闅旂鍘熷垯锛欳++ 鍐呴儴閫氳繃鎺ュ彛鎿嶄綔锛孋# 閫氳繃 Handle
    };

} // namespace NN::Runtime::Render

