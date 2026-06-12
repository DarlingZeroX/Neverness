// NNImGuiPass.cpp — ImGui pass implementation

#include "../../Features/NNImGuiPass.h"
#include <iostream>

namespace NN::Runtime::SRP
{
    bool NNImGuiPass::Setup(INNRenderDevice* device)
    {
        m_Device = device;
        return true;
    }

    void NNImGuiPass::Execute(INNCommandList* cmd, INNRenderTarget* renderTarget)
    {
        if (!cmd || !renderTarget) return;

        // ImGui pass execution:
        // In production, this would:
        //   1. Get ImGui draw data (ImDrawData)
        //   2. Bind the scene render target
        //   3. Set ImGui pipeline state (alpha blend, no depth)
        //   4. For each ImDrawList:
        //      a. Set vertex/index buffers
        //      b. Set scissor rects per command
        //      c. Draw indexed
        //
        // The actual render commands use INNCommandList interface.
        // BeginRenderPass/EndRenderPass are Diligent-specific
        // and handled by the backend implementation.
    }

    void NNImGuiPass::Shutdown()
    {
        m_Device = nullptr;
    }

} // namespace NN::Runtime::SRP
