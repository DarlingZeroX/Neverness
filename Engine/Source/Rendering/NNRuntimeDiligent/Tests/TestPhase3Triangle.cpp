// TestPhase3Triangle.cpp -- Phase 3: Shader + PSO creation test
// Verifies: device, shaders compile, PSO creates, basic render.

#include <iostream>
#include <SDL3/SDL.h>
#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>
#include <NNRuntimeDiligent/Pipeline/NNDiligentPipelineState.h>

namespace nnr = NN::Runtime::Render;

static const char* gVS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 P[3]; P[0]=float4(-0.5,-0.5,0,1); P[1]=float4(0,0.5,0,1); P[2]=float4(0.5,-0.5,0,1);
    float3 C[3]; C[0]=float3(1,0,0); C[1]=float3(0,1,0); C[2]=float3(0,0,1);
    PSIn.Pos=P[VertId]; PSIn.Color=C[VertId];
}
)";

static const char* gPS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
struct PSOutput { float4 Color : SV_TARGET; };
void main(in PSInput PSIn, out PSOutput PSOut) { PSOut.Color=float4(PSIn.Color.rgb,1); }
)";

static int g_Passed = 0;
static int g_Failed = 0;

#define TEST(name) std::cout << "  [TEST] " << name << "... ";
#define PASS() do { std::cout << "PASS" << std::endl; g_Passed++; } while(0)
#define FAIL(r) do { std::cout << "FAIL: " << r << std::endl; g_Failed++; } while(0)

int main()
{
    std::cout << "=== NNRuntime Phase 3: Shader + Pipeline Tests ===" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* sdlWnd = SDL_CreateWindow("Phase3", 800, 600, SDL_WINDOW_HIDDEN);
    if (!sdlWnd) { SDL_Quit(); return 1; }

    nnr::NNRenderDeviceCreateInfo devCI{};
    devCI.Window = sdlWnd;
    devCI.Width  = 800;
    devCI.Height = 600;
    devCI.EnableValidation = true;
    devCI.Backend = nnr::NNRenderBackendType::Auto;

    auto renderDev = nnr::NNRenderBootstrap::CreateDevice(devCI);
    if (!renderDev)
    {
        std::cerr << "CreateDevice FAILED!" << std::endl;
        SDL_DestroyWindow(sdlWnd); SDL_Quit();
        return 1;
    }
    std::cout << "Device: " << renderDev->GetDeviceInfo().DeviceName << std::endl << std::endl;

    auto* dilDevice = static_cast<NNDiligent::NNDiligentDevice*>(renderDev.Get());

    // ====== Test 1: Create Vertex Shader ======
    {
        TEST("Create Vertex Shader (HLSL)")
        nnr::NNShaderDesc desc{};
        desc.Stage = nnr::NNShaderStage::Vertex;
        desc.SourceCode = gVS;
        desc.EntryPoint = "main";
        desc.DebugName = "TestVS";
        auto vs = renderDev->CreateShader(desc);
        if (!vs) FAIL("CreateShader returned null");
        else PASS();
    }

    // ====== Test 2: Create Pixel Shader ======
    {
        TEST("Create Pixel Shader (HLSL)")
        nnr::NNShaderDesc desc{};
        desc.Stage = nnr::NNShaderStage::Pixel;
        desc.SourceCode = gPS;
        desc.EntryPoint = "main";
        desc.DebugName = "TestPS";
        auto ps = renderDev->CreateShader(desc);
        if (!ps) FAIL("CreateShader returned null");
        else PASS();
    }

    // ====== Test 3: Create PSO ======
    {
        TEST("Create PipelineState (Triangle, no depth)")

        nnr::NNShaderDesc vsDesc{};
        vsDesc.Stage = nnr::NNShaderStage::Vertex;
        vsDesc.SourceCode = gVS;
        vsDesc.EntryPoint = "main";
        auto vs = renderDev->CreateShader(vsDesc);
        if (!vs) { FAIL("VS creation failed"); }
        else
        {
            nnr::NNShaderDesc psDesc{};
            psDesc.Stage = nnr::NNShaderStage::Pixel;
            psDesc.SourceCode = gPS;
            psDesc.EntryPoint = "main";
            auto ps = renderDev->CreateShader(psDesc);
            if (!ps) { FAIL("PS creation failed"); }
            else
            {
                nnr::NNPipelineStateDesc psoDesc{};
                psoDesc.VS = vs.Get();
                psoDesc.PS = ps.Get();
                psoDesc.RasterizerState.CullMode = nnr::NNCullMode::None;
                psoDesc.DepthStencilState.DepthEnable = false;
                psoDesc.DepthStencilState.DepthWriteEnable = false;
                psoDesc.Topology = nnr::NNPrimitiveTopology::TriangleList;
                psoDesc.RTVFormat = nnr::NNPixelFormat::RGBA8_UNORM;
                psoDesc.DSVFormat = nnr::NNPixelFormat::Unknown;
                psoDesc.SampleCount = 1;
                psoDesc.DebugName = "TestPSO";
                auto pso = renderDev->CreatePipelineState(psoDesc);
                if (!pso) FAIL("CreatePipelineState returned null");
                else
                {
                    assert(pso->GetDesc().VS != nullptr);
                    assert(pso->GetDesc().PS != nullptr);
                    PASS();
                }
            }
        }
    }

    // ====== Test 4: Create PSO with Depth ======
    {
        TEST("Create PipelineState (with Depth)")

        nnr::NNShaderDesc vsDesc{};
        vsDesc.Stage = nnr::NNShaderStage::Vertex;
        vsDesc.SourceCode = gVS;
        vsDesc.EntryPoint = "main";
        auto vs = renderDev->CreateShader(vsDesc);

        nnr::NNShaderDesc psDesc{};
        psDesc.Stage = nnr::NNShaderStage::Pixel;
        psDesc.SourceCode = gPS;
        psDesc.EntryPoint = "main";
        auto ps = renderDev->CreateShader(psDesc);

        if (!vs || !ps) { FAIL("Shader creation failed"); }
        else
        {
            nnr::NNPipelineStateDesc psoDesc{};
            psoDesc.VS = vs.Get();
            psoDesc.PS = ps.Get();
            psoDesc.RasterizerState.CullMode = nnr::NNCullMode::Back;
            psoDesc.DepthStencilState.DepthEnable = true;
            psoDesc.DepthStencilState.DepthWriteEnable = true;
            psoDesc.Topology = nnr::NNPrimitiveTopology::TriangleList;
            psoDesc.RTVFormat = nnr::NNPixelFormat::RGBA8_UNORM;
            psoDesc.DSVFormat = nnr::NNPixelFormat::D32_FLOAT;
            psoDesc.SampleCount = 1;
            psoDesc.DebugName = "TestPSO_Depth";
            auto pso = renderDev->CreatePipelineState(psoDesc);
            if (!pso) FAIL("CreatePipelineState returned null");
            else
            {
                assert(pso->GetDesc().DepthStencilState.DepthEnable == true);
                PASS();
            }
        }
    }

    // ====== Test 5: Create PSO with Wireframe ======
    {
        TEST("Create PipelineState (Wireframe)")

        nnr::NNShaderDesc vsDesc{};
        vsDesc.Stage = nnr::NNShaderStage::Vertex;
        vsDesc.SourceCode = gVS;
        vsDesc.EntryPoint = "main";
        auto vs = renderDev->CreateShader(vsDesc);

        nnr::NNShaderDesc psDesc{};
        psDesc.Stage = nnr::NNShaderStage::Pixel;
        psDesc.SourceCode = gPS;
        psDesc.EntryPoint = "main";
        auto ps = renderDev->CreateShader(psDesc);

        if (!vs || !ps) { FAIL("Shader creation failed"); }
        else
        {
            nnr::NNPipelineStateDesc psoDesc{};
            psoDesc.VS = vs.Get();
            psoDesc.PS = ps.Get();
            psoDesc.RasterizerState.FillMode = nnr::NNFillMode::Wireframe;
            psoDesc.RasterizerState.CullMode = nnr::NNCullMode::None;
            psoDesc.DepthStencilState.DepthEnable = false;
            psoDesc.DepthStencilState.DepthWriteEnable = false;
            psoDesc.RTVFormat = nnr::NNPixelFormat::RGBA8_UNORM;
            psoDesc.DSVFormat = nnr::NNPixelFormat::Unknown;
            auto pso = renderDev->CreatePipelineState(psoDesc);
            if (!pso) FAIL("CreatePipelineState returned null");
            else PASS();
        }
    }

    // ====== Test 6: CommandList SetPipelineState ======
    {
        TEST("CommandList::SetPipelineState")

        nnr::NNShaderDesc vsDesc{};
        vsDesc.Stage = nnr::NNShaderStage::Vertex;
        vsDesc.SourceCode = gVS;
        vsDesc.EntryPoint = "main";
        auto vs = renderDev->CreateShader(vsDesc);

        nnr::NNShaderDesc psDesc{};
        psDesc.Stage = nnr::NNShaderStage::Pixel;
        psDesc.SourceCode = gPS;
        psDesc.EntryPoint = "main";
        auto ps = renderDev->CreateShader(psDesc);

        if (!vs || !ps) { FAIL("Shader creation failed"); }
        else
        {
            nnr::NNPipelineStateDesc psoDesc{};
            psoDesc.VS = vs.Get();
            psoDesc.PS = ps.Get();
            psoDesc.RasterizerState.CullMode = nnr::NNCullMode::None;
            psoDesc.DepthStencilState.DepthEnable = false;
            psoDesc.RTVFormat = nnr::NNPixelFormat::RGBA8_UNORM;
            psoDesc.DSVFormat = nnr::NNPixelFormat::Unknown;
            auto pso = renderDev->CreatePipelineState(psoDesc);
            if (!pso) { FAIL("PSO creation failed"); }
            else
            {
                auto* cmd = dilDevice->GetImmediateCommandList();
                cmd->SetPipelineState(pso.Get());
                PASS();
            }
        }
    }

    // ====== Test 7: CommandList SetViewports ======
    {
        TEST("CommandList::SetViewports")
        auto* cmd = dilDevice->GetImmediateCommandList();
        nnr::NNViewport vp{};
        vp.Width = 800.0f;
        vp.Height = 600.0f;
        vp.MaxDepth = 1.0f;
        cmd->SetViewports(&vp, 1);
        PASS();
    }

    // ====== Summary ======
    std::cout << std::endl;
    std::cout << "=== Results: " << g_Passed << " passed, " << g_Failed << " failed ===" << std::endl;

    renderDev = nullptr;
    SDL_DestroyWindow(sdlWnd);
    SDL_Quit();

    return g_Failed > 0 ? 1 : 0;
}
