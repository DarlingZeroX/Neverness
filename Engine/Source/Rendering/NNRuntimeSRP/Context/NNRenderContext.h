#pragma once

#include "NNCameraData.h"
#include "NNSceneData.h"
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRenderAssets/Cache/NNPipelineCache.h>
#include <NNRuntimeRenderAssets/Cache/NNAssetRegistry.h>
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Assets;

    // ========================================================================
    //  NNRenderContext â€?Per-frame rendering context
    //  Passed to pipeline Execute(), contains all rendering state
    // ========================================================================

    struct NNRenderContext
    {
        // Camera
        NNCameraData Camera;

        // Scene (lights, environment)
        NNSceneData Scene;

        // Device and caches (not owned)
        INNRenderDevice* Device = nullptr;
        NNPipelineCache* PipelineCache = nullptr;
        NNAssetRegistry* AssetRegistry = nullptr;

        // Frame dimensions
        uint32_t FrameWidth = 0;
        uint32_t FrameHeight = 0;

        // Frame counter
        uint64_t FrameNumber = 0;

        // Delta time (seconds)
        float DeltaTime = 0.0f;

        // Total time (seconds)
        float TotalTime = 0.0f;

        // Validate context
        bool IsValid() const
        {
            return Device != nullptr && PipelineCache != nullptr && FrameWidth > 0 && FrameHeight > 0;
        }
    };

} // namespace NN::Runtime::SRP
