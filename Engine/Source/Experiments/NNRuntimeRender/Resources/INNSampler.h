#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Render
{
    // using namespace removed - use fully qualified names

    // ========================================================================
    //  Sampler 鎺ュ彛
    // ========================================================================

    enum class NNFilterMode : uint8_t
    {
        Point,
        Linear,
        Anisotropic
    };

    enum class NNAddressMode : uint8_t
    {
        Wrap,
        Clamp,
        Mirror,
        Border
    };

    struct NNSamplerDesc
    {
        NNFilterMode MinFilter = NNFilterMode::Linear;
        NNFilterMode MagFilter = NNFilterMode::Linear;
        NNFilterMode MipFilter = NNFilterMode::Linear;
        NNAddressMode AddressU = NNAddressMode::Wrap;
        NNAddressMode AddressV = NNAddressMode::Wrap;
        NNAddressMode AddressW = NNAddressMode::Wrap;
        uint32_t MaxAnisotropy = 16;
        float MipLODBias = 0.0f;
        float MinLOD = 0.0f;
        float MaxLOD = 1000.0f;
    };

    class INNSampler : public NN::Runtime::Core::INNObject
    {
    public:
        virtual const NNSamplerDesc& GetDesc() const = 0;
    };

} // namespace NN::Runtime::Render

