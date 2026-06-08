#pragma once

#include "../Context/NNRenderContext.h"
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeCore/NNObject.h>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;

    // ========================================================================
    //  NNImGuiPass â€?ImGui rendering pass
    //  Renders ImGui draw data over the scene
    // ========================================================================

    class NNImGuiPass
    {
    public:
        NNImGuiPass() = default;
        ~NNImGuiPass() = default;

        // Setup (connect to existing ImGui context)
        bool Setup(INNRenderDevice* device);

        // Execute ImGui pass (renders into provided RT)
        void Execute(INNCommandList* cmd, INNRenderTarget* renderTarget);

        // Shutdown
        void Shutdown();

    private:
        INNRenderDevice* m_Device = nullptr;
    };

} // namespace NN::Runtime::SRP
