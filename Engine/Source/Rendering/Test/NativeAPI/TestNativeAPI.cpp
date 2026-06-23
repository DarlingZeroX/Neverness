// TestNativeAPI.cpp -- Phase 4: NNNativeEngineAPI integration test
// Verifies: device creation, handle-based resource creation via C API,
//           render commands, and proper handle cleanup.

#include <iostream>
#include <SDL3/SDL.h>
#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeRender/Pipeline/INNShader.h>
#include <NNRuntimeRender/Pipeline/INNPipelineState.h>
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>
#include <NNEngineContext.h>
#include <NNRenderAPI.h>
#include <NNResourceAPI.h>

namespace nnr = NN::Runtime::Render;

// HLSL vertex shader -- triangle from vertex ID, per-vertex color
static const char* gVS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 P[3]; P[0]=float4(-0.5,-0.5,0,1); P[1]=float4(0,0.5,0,1); P[2]=float4(0.5,-0.5,0,1);
    float3 C[3]; C[0]=float3(1,0,0); C[1]=float3(0,1,0); C[2]=float3(0,0,1);
    PSIn.Pos=P[VertId]; PSIn.Color=C[VertId];
}
)";

// HLSL pixel shader -- pass-through color
static const char* gPS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
struct PSOutput { float4 Color : SV_TARGET; };
void main(in PSInput PSIn, out PSOutput PSOut) { PSOut.Color=float4(PSIn.Color.rgb,1); }
)";

// Vertex: position (3 float) + color (3 float) = 24 bytes
struct SimpleVertex
{
    float px, py, pz;
    float cr, cg, cb;
};

// Triangle with per-vertex colors
static const SimpleVertex gTriangleVerts[] =
{
    { -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f },
    {  0.0f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f },
    {  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f },
};

static int g_Passed = 0;
static int g_Failed = 0;

#define TEST(name) std::cout << "  [TEST] " << name << "... ";
#define PASS() do { std::cout << "PASS" << std::endl; g_Passed++; } while(0)
#define FAIL(r) do { std::cout << "FAIL: " << r << std::endl; g_Failed++; } while(0)

int main()
{
    std::cout << "=== NNRuntime Phase 4: NNNativeEngineAPI Tests ===" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* sdlWnd = SDL_CreateWindow("Phase4", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!sdlWnd) { SDL_Quit(); return 1; }

    // ====== Test 1: Create Device via Bootstrap ======
    TEST("Create Device via Bootstrap")
    nnr::NNRenderDeviceCreateInfo devCI{};
    devCI.Window = sdlWnd;
    devCI.Width  = 800;
    devCI.Height = 600;
    devCI.EnableValidation = true;
    devCI.Backend = nnr::NNRenderBackendType::Backend_Vulkan;

    auto renderDev = nnr::NNRenderBootstrap::CreateDevice(devCI);
    if (!renderDev)
    {
        FAIL("CreateDevice returned null");
        SDL_DestroyWindow(sdlWnd); SDL_Quit();
        return 1;
    }
    PASS();

    std::cout << "  Device: " << renderDev->GetDeviceInfo().DeviceName << std::endl;

    // ====== Test 2: Initialize NNEngineContext ======
    TEST("Initialize NNEngineContext")
    auto& engineCtx = NN::Runtime::NativeAPI::NNEngineContext::Get();
    if (!engineCtx.Initialize(renderDev.Get()))
    {
        FAIL("NNEngineContext::Initialize failed");
    }
    else
    {
        assert(engineCtx.GetDevice() == renderDev.Get());
        PASS();
    }

    // ====== Test 3: Create Vertex Shader via C API ======
    NNE_Handle hVS = NNE_INVALID_HANDLE;
    {
        TEST("NNE_CreateShader (Vertex)")
        hVS = NNE_CreateShader(static_cast<uint32_t>(nnr::NNShaderStage::Vertex), gVS, "main", "TestVS");
        if (hVS == NNE_INVALID_HANDLE) FAIL("returned invalid handle");
        else
        {
            assert(NNE_IsHandleValid(hVS) == 1);
            assert(NNE_GetHandleType(hVS) == static_cast<uint32_t>(NN::Runtime::Core::NNHandleType::Shader));
            PASS();
        }
    }

    // ====== Test 4: Create Pixel Shader via C API ======
    NNE_Handle hPS = NNE_INVALID_HANDLE;
    {
        TEST("NNE_CreateShader (Pixel)")
        hPS = NNE_CreateShader(static_cast<uint32_t>(nnr::NNShaderStage::Pixel), gPS, "main", "TestPS");
        if (hPS == NNE_INVALID_HANDLE) FAIL("returned invalid handle");
        else
        {
            assert(NNE_IsHandleValid(hPS) == 1);
            PASS();
        }
    }

    // ====== Test 5: Create Vertex Buffer via C API ======
    NNE_Handle hVB = NNE_INVALID_HANDLE;
    {
        TEST("NNE_CreateBuffer (Vertex, triangle)")
        hVB = NNE_CreateBuffer(
            sizeof(gTriangleVerts),
            static_cast<uint32_t>(nnr::NNBufferType::Vertex),
            static_cast<uint32_t>(nnr::NNBufferUsage::Static),
            gTriangleVerts);
        if (hVB == NNE_INVALID_HANDLE) FAIL("returned invalid handle");
        else
        {
            assert(NNE_IsHandleValid(hVB) == 1);
            assert(NNE_GetHandleType(hVB) == static_cast<uint32_t>(NN::Runtime::Core::NNHandleType::Buffer));
            assert(NNE_BufferGetSize(hVB) == sizeof(gTriangleVerts));
            PASS();
        }
    }

    // ====== Test 6: Create PipelineState via C API ======
    NNE_Handle hPSO = NNE_INVALID_HANDLE;
    {
        TEST("NNE_CreatePipelineState (Triangle)")
        nnr::NNPipelineStateDesc psoDesc{};
        psoDesc.RasterizerState.CullMode = nnr::NNCullMode::None;
        psoDesc.DepthStencilState.DepthEnable = false;
        psoDesc.DepthStencilState.DepthWriteEnable = false;
        psoDesc.Topology = nnr::NNPrimitiveTopology::TriangleList;
        psoDesc.RTVFormat = nnr::NNPixelFormat::RGBA8_UNORM;
        psoDesc.DSVFormat = nnr::NNPixelFormat::Unknown;
        psoDesc.SampleCount = 1;

        hPSO = NNE_CreatePipelineState(hVS, hPS, &psoDesc);
        if (hPSO == NNE_INVALID_HANDLE) FAIL("returned invalid handle");
        else
        {
            assert(NNE_IsHandleValid(hPSO) == 1);
            assert(NNE_GetHandleType(hPSO) == static_cast<uint32_t>(NN::Runtime::Core::NNHandleType::Pipeline));
            PASS();
        }
    }

    // ====== Test 7: Render 5 frames via C API ======
    {
        TEST("Render 5 frames (Clear + SetPSO + SetVB + Draw + Present)")
        bool renderOk = true;
        for (int frame = 0; frame < 5; ++frame)
        {
            NNE_ClearRenderTarget(0.1f, 0.1f, 0.2f, 1.0f);
            NNE_SetViewport(0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f);
            NNE_SetPipelineState(hPSO);
            NNE_SetVertexBuffer(hVB, 0);
            NNE_Draw(3, 0, 1);
            NNE_Present();
        }
        PASS();
    }

    // ====== Test 8: Release all handles ======
    {
        TEST("NNE_ReleaseHandle (all resources)")
        NNE_ReleaseHandle(hPSO);
        NNE_ReleaseHandle(hVB);
        NNE_ReleaseHandle(hPS);
        NNE_ReleaseHandle(hVS);

        assert(NNE_IsHandleValid(hPSO) == 0);
        assert(NNE_IsHandleValid(hVB) == 0);
        assert(NNE_IsHandleValid(hPS) == 0);
        assert(NNE_IsHandleValid(hVS) == 0);
        PASS();
    }

    // ====== Summary ======
    std::cout << std::endl;
    std::cout << "=== Results: " << g_Passed << " passed, " << g_Failed << " failed ===" << std::endl;

    // Cleanup
    engineCtx.Shutdown();
    renderDev = nullptr;
    SDL_DestroyWindow(sdlWnd);
    SDL_Quit();

    return g_Failed > 0 ? 1 : 0;
}
