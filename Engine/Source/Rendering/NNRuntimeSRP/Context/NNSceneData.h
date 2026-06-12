#pragma once

#include "NNSRPMath.h"
#include <vector>
#include <cstdint>

namespace NN::Runtime::SRP
{
    // ========================================================================
    //  NNLightData â€?Light parameters
    // ========================================================================

    enum class NNLightType : uint8_t
    {
        Directional,
        Point,
        Spot
    };

    struct NNLightData
    {
        NNLightType Type = NNLightType::Directional;

        // Direction (for directional/spot)
        Vector3 Direction = {0.0f, -1.0f, 0.0f};

        // Position (for point/spot)
        Vector3 Position = {0.0f, 0.0f, 0.0f};

        // Color and intensity
        Vector3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;

        // Shadow parameters
        bool CastShadows = true;
        float ShadowBias = 0.005f;
        float ShadowNormalBias = 0.02f;
        uint32_t ShadowMapSize = 2048;

        // Light view-projection for shadow mapping
        Matrix4x4 LightViewProj;

        // Setup directional light shadow camera
        void SetupShadowCamera(const Vector3& sceneCenter, float sceneRadius)
        {
            Vector3 lightDir = Direction.Normalized();
            Vector3 lightPos = sceneCenter - lightDir * sceneRadius;

            // Ortho projection covering the scene
            float s = sceneRadius;
            LightViewProj = Matrix4x4::LookAt(lightPos, sceneCenter, {0, 1, 0})
                          * Matrix4x4::Ortho(-s, s, -s, s, 0.1f, sceneRadius * 2.0f);
        }
    };

    // ========================================================================
    //  NNAmbientData â€?Ambient/environment settings
    // ========================================================================

    struct NNAmbientData
    {
        Vector3 SkyColor = {0.5f, 0.7f, 1.0f};
        float SkyIntensity = 0.3f;
        Vector3 GroundColor = {0.2f, 0.15f, 0.1f};
        float GroundIntensity = 0.1f;
    };

    // ========================================================================
    //  NNSceneData â€?Complete scene data for rendering
    // ========================================================================

    struct NNSceneData
    {
        std::vector<NNLightData> Lights;
        NNAmbientData Ambient;

        // Add a directional light
        void AddDirectionalLight(const Vector3& dir, const Vector3& color, float intensity)
        {
            NNLightData light;
            light.Type = NNLightType::Directional;
            light.Direction = dir.Normalized();
            light.Color = color;
            light.Intensity = intensity;
            Lights.push_back(light);
        }

        // Get main directional light (first one)
        const NNLightData* GetMainLight() const
        {
            for (const auto& light : Lights)
            {
                if (light.Type == NNLightType::Directional) return &light;
            }
            return nullptr;
        }
    };

} // namespace NN::Runtime::SRP
