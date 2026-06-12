// NNVGFXAdapter.h -- Maps old VGFX rendering style to NNRuntimeRender interfaces
// This is a compatibility layer for migrating existing code.

#pragma once

#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>

namespace NNDiligent
{
    // High-level rendering helpers that use NNRuntimeRender interfaces.
    // Similar to VGFX API style but backed by Diligent.
    class NNVGFXAdapter
    {
    public:
        NNVGFXAdapter(NN::Runtime::Render::INNRenderDevice* device);
        ~NNVGFXAdapter();

        // Simple draw helpers (like VGFX)
        bool DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
                          float r, float g, float b);
        bool DrawQuad(float x, float y, float w, float h,
                      NN::Runtime::Core::NNRef<NN::Runtime::Render::INNTexture> texture);

    private:
        NN::Runtime::Render::INNRenderDevice* m_Device;
        // Cached PSOs for common operations
        NN::Runtime::Core::NNRef<NN::Runtime::Render::INNPipelineState> m_ColorPSO;
    };

} // namespace NNDiligent
