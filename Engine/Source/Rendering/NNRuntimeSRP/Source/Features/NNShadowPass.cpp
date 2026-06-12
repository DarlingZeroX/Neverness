// NNShadowPass.cpp — Shadow pass implementation

#include "../../Features/NNShadowPass.h"
#include <iostream>

namespace NN::Runtime::SRP
{
    bool NNShadowPass::Setup(INNRenderDevice* device, uint32_t shadowMapSize)
    {
        if (!device) return false;

        m_ShadowMapSize = shadowMapSize;

        // Create shadow map render target (depth only)
        NNRenderTargetDesc desc;
        desc.Width = shadowMapSize;
        desc.Height = shadowMapSize;
        desc.ColorFormat = NNPixelFormat::Unknown; // Depth-only
        desc.DepthFormat = NNPixelFormat::D32_FLOAT;
        desc.DebugName = "ShadowMap";

        m_ShadowRT = device->CreateRenderTarget(desc);
        if (!m_ShadowRT)
        {
            std::cerr << "[ShadowPass] Failed to create shadow map RT" << std::endl;
            return false;
        }

        std::cout << "[ShadowPass] Shadow map created: " << shadowMapSize << "x" << shadowMapSize << std::endl;
        return true;
    }

    void NNShadowPass::Execute(INNCommandList* cmd, const NNRenderContext& ctx)
    {
        if (!cmd || !m_ShadowRT) return;

        const auto* mainLight = ctx.Scene.GetMainLight();
        if (!mainLight || !mainLight->CastShadows) return;

        // Calculate light view-projection
        Vector3 lightDir = mainLight->Direction.Normalized();
        Vector3 sceneCenter = {0, 0, 0};
        float sceneRadius = 10.0f;

        Vector3 lightPos = sceneCenter - lightDir * sceneRadius;
        m_LightViewProj = Matrix4x4::LookAt(lightPos, sceneCenter, {0, 1, 0})
                        * Matrix4x4::Ortho(-sceneRadius, sceneRadius,
                                           -sceneRadius, sceneRadius,
                                           0.1f, sceneRadius * 2.0f);

        // Shadow pass execution:
        // In production, this would:
        //   1. Bind shadow render target
        //   2. Set viewport to shadow map size
        //   3. Set shadow material (depth-only shader)
        //   4. Set light camera matrices as constant buffer
        //   5. Draw all shadow casters
        //
        // The actual render commands (SetViewports, Draw, etc.)
        // are called through the INNCommandList interface.
        // BeginRenderPass/EndRenderPass are Diligent-specific
        // and handled by the backend implementation.

        // For now, this is a structural placeholder.
        // The shadow map RT is set up and light matrices computed.
    }

} // namespace NN::Runtime::SRP
