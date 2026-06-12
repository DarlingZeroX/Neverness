#pragma once

#include "../Context/NNRenderContext.h"
#include "../Pipeline/NNPipelinePass.h"
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeRenderAssets/Material/NNMaterial.h>
#include <NNRuntimeCore/NNObject.h>
#include <vector>

namespace NN::Runtime::SRP
{
    using namespace NN::Runtime::Render;
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Assets;

    // ========================================================================
    //  NNShadowPass â€?Shadow map generation
    //  Renders scene depth from light's perspective
    // ========================================================================

    class NNShadowPass
    {
    public:
        NNShadowPass() = default;
        ~NNShadowPass() = default;

        // Setup shadow render target
        bool Setup(INNRenderDevice* device, uint32_t shadowMapSize = 2048);

        // Execute shadow pass
        // Draws all shadow-casting objects from the light's POV
        void Execute(INNCommandList* cmd, const NNRenderContext& ctx);

        // Get shadow map render target
        NNRef<INNRenderTarget> GetShadowMap() const { return m_ShadowRT; }

        // Get light view-projection matrix
        const Matrix4x4& GetLightViewProj() const { return m_LightViewProj; }

        // Settings
        void SetShadowMapSize(uint32_t size) { m_ShadowMapSize = size; }
        uint32_t GetShadowMapSize() const { return m_ShadowMapSize; }

    private:
        NNRef<INNRenderTarget> m_ShadowRT;
        Matrix4x4 m_LightViewProj;
        uint32_t m_ShadowMapSize = 2048;
    };

} // namespace NN::Runtime::SRP
