// NNForwardPipeline.cpp — Forward rendering pipeline implementation

#include "../../Pipeline/NNForwardPipeline.h"
#include <iostream>

namespace NN::Runtime::SRP
{
    NNForwardPipeline::NNForwardPipeline()
    {
    }

    bool NNForwardPipeline::Initialize(INNRenderDevice* device, uint32_t width, uint32_t height)
    {
        if (!device) return false;

        m_Device = device;
        m_Width = width;
        m_Height = height;

        std::cout << "[ForwardPipeline] Initializing (" << width << "x" << height << ")" << std::endl;

        // Setup shadow pass
        if (!m_ShadowPass.Setup(device))
        {
            std::cerr << "[ForwardPipeline] Shadow pass setup failed" << std::endl;
            return false;
        }

        // Setup forward pass
        if (!m_ForwardPass.Setup(device, width, height))
        {
            std::cerr << "[ForwardPipeline] Forward pass setup failed" << std::endl;
            return false;
        }

        // Setup ImGui pass
        if (!m_ImGuiPass.Setup(device))
        {
            std::cerr << "[ForwardPipeline] ImGui pass setup failed" << std::endl;
            return false;
        }

        // Create pipeline pass descriptors
        NNPipelinePassDesc shadowDesc;
        shadowDesc.Name = "Shadow";
        shadowDesc.Type = NNPassType::Shadow;
        shadowDesc.RTVFormat = NNPixelFormat::D32_FLOAT;
        shadowDesc.DSVFormat = NNPixelFormat::D32_FLOAT;
        shadowDesc.Width = m_ShadowPass.GetShadowMapSize();
        shadowDesc.Height = m_ShadowPass.GetShadowMapSize();
        shadowDesc.ClearColor = false;
        shadowDesc.ClearDepth = true;

        NNPipelinePassDesc forwardDesc;
        forwardDesc.Name = "Forward";
        forwardDesc.Type = NNPassType::Forward;
        forwardDesc.RTVFormat = NNPixelFormat::RGBA8_UNORM;
        forwardDesc.DSVFormat = NNPixelFormat::D32_FLOAT;
        forwardDesc.Width = width;
        forwardDesc.Height = height;
        forwardDesc.ClearColor = true;
        forwardDesc.ClearR = 0.1f;
        forwardDesc.ClearG = 0.1f;
        forwardDesc.ClearB = 0.2f;
        forwardDesc.ClearA = 1.0f;

        NNPipelinePassDesc imguiDesc;
        imguiDesc.Name = "ImGui";
        imguiDesc.Type = NNPassType::ImGui;
        imguiDesc.RTVFormat = NNPixelFormat::RGBA8_UNORM;
        imguiDesc.DSVFormat = NNPixelFormat::D32_FLOAT;
        imguiDesc.Width = width;
        imguiDesc.Height = height;
        imguiDesc.ClearColor = false;
        imguiDesc.ClearDepth = false;

        m_Passes.emplace_back(shadowDesc);
        m_Passes.emplace_back(forwardDesc);
        m_Passes.emplace_back(imguiDesc);

        std::cout << "[ForwardPipeline] Initialized with " << m_Passes.size() << " passes" << std::endl;
        return true;
    }

    void NNForwardPipeline::Execute(INNCommandList* cmd, const NNRenderContext& ctx)
    {
        if (!cmd || !ctx.IsValid()) return;

        // === Pass 1: Shadow ===
        m_ShadowPass.Execute(cmd, ctx);

        // === Pass 2: Forward ===
        // In real usage, the scene would provide draw calls
        // For now, just execute the pass structure
        std::vector<NNForwardDrawCall> drawCalls; // Empty for test
        m_ForwardPass.Execute(cmd, ctx, drawCalls);

        // === Pass 3: ImGui ===
        // Would render ImGui draw data here
        // m_ImGuiPass.Execute(cmd, m_ForwardPass.GetColorTarget());
    }

    void NNForwardPipeline::Shutdown()
    {
        std::cout << "[ForwardPipeline] Shutdown" << std::endl;

        m_ImGuiPass.Shutdown();
        m_Passes.clear();
        m_Device = nullptr;
    }

    void NNForwardPipeline::OnResize(uint32_t width, uint32_t height)
    {
        if (width == m_Width && height == m_Height) return;

        m_Width = width;
        m_Height = height;

        if (m_Device)
        {
            m_ForwardPass.OnResize(m_Device, width, height);

            // Update pass descriptors
            for (auto& pass : m_Passes)
            {
                if (pass.GetDesc().Type == NNPassType::Forward ||
                    pass.GetDesc().Type == NNPassType::ImGui)
                {
                    // Would need to recreate pass targets
                }
            }
        }
    }

} // namespace NN::Runtime::SRP
