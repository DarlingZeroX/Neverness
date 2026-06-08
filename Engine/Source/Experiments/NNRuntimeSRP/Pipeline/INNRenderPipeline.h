#pragma once

#include "../NNRenderConfig.h"
#include "../Context/NNRenderContext.h"
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  INNRenderPipeline â€?Abstract render pipeline interface
    //  C# can inherit and override to create custom rendering pipelines
    // ========================================================================

    class INNRenderPipeline : public INNObject
    {
    public:
        // Initialize pipeline (create passes, RTs)
        virtual bool Initialize(INNRenderDevice* device, uint32_t width, uint32_t height) = 0;

        // Execute pipeline for one frame
        virtual void Execute(INNCommandList* cmd, const NNRenderContext& ctx) = 0;

        // Shutdown and release resources
        virtual void Shutdown() = 0;

        // Get pipeline name
        virtual const char* GetName() const = 0;

        // Get number of passes
        virtual uint32_t GetPassCount() const = 0;

        // Resize (recreate RTs)
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
    };

} // namespace NN::Runtime::SRP
