// =============================================================================
// TestPhase1.cpp
// Phase 1: SDL3 窗口 + Diligent 渲染 + Smoke Test
//
// 验证内容:
//   - SDL3 窗口创建
//   - Diligent D3D12 设备初始化
//   - 渲染彩色三角形（Smoke Test）
// =============================================================================

#include <iostream>
#include <SDL3/SDL.h>

// Diligent 头文件
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/RenderPass.h"
#include "Graphics/GraphicsEngine/interface/Framebuffer.h"

// D3D12 工厂
#ifdef D3D12_SUPPORTED
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#endif

// D3D11 工厂
#ifdef D3D11_SUPPORTED
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#endif

// 简单三角形 Shader（使用 SV_VertexID）
static const char* gTriangleVS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 P[3]; P[0]=float4(-0.5,-0.5,0,1); P[1]=float4(0,0.5,0,1); P[2]=float4(0.5,-0.5,0,1);
    float3 C[3]; C[0]=float3(1,0,0); C[1]=float3(0,1,0); C[2]=float3(0,0,1);
    PSIn.Pos=P[VertId]; PSIn.Color=C[VertId];
}
)";

static const char* gTrianglePS = R"(
struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
struct PSOutput { float4 Color : SV_TARGET; };
void main(in PSInput PSIn, out PSOutput PSOut) { PSOut.Color=float4(PSIn.Color.rgb,1); }
)";

// 创建 Diligent 设备
static bool CreateDeviceAndSwapChain(
    SDL_Window* window,
    Diligent::IRenderDevice** ppDevice,
    Diligent::IDeviceContext** ppContext,
    Diligent::ISwapChain** ppSwapChain)
{
    // 获取 SDL 窗口信息
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    void* hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!hwnd) {
        std::cerr << "Failed to get HWND from SDL window" << std::endl;
        return false;
    }

    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);

    Diligent::SwapChainDesc scDesc;
    scDesc.Width = static_cast<Diligent::Uint32>(width);
    scDesc.Height = static_cast<Diligent::Uint32>(height);
    scDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    scDesc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT;
    scDesc.Usage = Diligent::SWAP_CHAIN_USAGE_RENDER_TARGET;

    // 尝试 D3D12
#ifdef D3D12_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D12();
        if (factory) {
            Diligent::EngineD3D12CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D12(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                Diligent::Win32NativeWindow nativeWnd{hwnd};
                factory->CreateSwapChainD3D12(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
                if (*ppSwapChain) {
                    std::cout << "[Phase1] Backend: D3D12" << std::endl;
                    return true;
                }
            }
        }
    }
#endif

    // 尝试 D3D11
#ifdef D3D11_SUPPORTED
    {
        auto* factory = Diligent::GetEngineFactoryD3D11();
        if (factory) {
            Diligent::EngineD3D11CreateInfo engineCI;
            engineCI.EnableValidation = true;
            factory->CreateDeviceAndContextsD3D11(engineCI, ppDevice, ppContext);
            if (*ppDevice) {
                Diligent::Win32NativeWindow nativeWnd{hwnd};
                factory->CreateSwapChainD3D11(*ppDevice, *ppContext, scDesc, Diligent::FullScreenModeDesc{}, nativeWnd, ppSwapChain);
                if (*ppSwapChain) {
                    std::cout << "[Phase1] Backend: D3D11" << std::endl;
                    return true;
                }
            }
        }
    }
#endif

    return false;
}

// 编译 Shader
static Diligent::RefCntAutoPtr<Diligent::IShader> CompileShader(
    Diligent::IRenderDevice* device,
    const char* source,
    Diligent::SHADER_TYPE type,
    const char* name)
{
    Diligent::ShaderCreateInfo sci;
    sci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    sci.Desc.UseCombinedTextureSamplers = false;
    sci.Desc.ShaderType = type;
    sci.EntryPoint = "main";
    sci.Desc.Name = name;
    sci.Source = source;
    sci.HLSLVersion = {5, 0};

    Diligent::RefCntAutoPtr<Diligent::IShader> shader;
    device->CreateShader(sci, &shader);

    if (shader) {
        std::cout << "  [OK] " << name << std::endl;
    } else {
        std::cerr << "  [FAIL] " << name << std::endl;
    }

    return shader;
}

int main(int argc, char* argv[])
{
    std::cout << "============================================" << std::endl;
    std::cout << "  RmlDiligent Phase 1: SDL3 + Smoke Test" << std::endl;
    std::cout << "============================================" << std::endl;

    // 初始化 SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init FAILED: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow(
        "RmlDiligent Phase 1",
        800, 600,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow FAILED: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] SDL3 窗口创建成功" << std::endl;

    // 创建 Diligent 设备
    Diligent::IRenderDevice* device = nullptr;
    Diligent::IDeviceContext* context = nullptr;
    Diligent::ISwapChain* swapChain = nullptr;

    if (!CreateDeviceAndSwapChain(window, &device, &context, &swapChain)) {
        std::cerr << "[FAIL] CreateDeviceAndSwapChain FAILED!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] Diligent 设备创建成功" << std::endl;

    // 编译 Shader
    auto vs = CompileShader(device, gTriangleVS, Diligent::SHADER_TYPE_VERTEX, "TriangleVS");
    auto ps = CompileShader(device, gTrianglePS, Diligent::SHADER_TYPE_PIXEL, "TrianglePS");

    if (!vs || !ps) {
        std::cerr << "[FAIL] Shader 编译失败!" << std::endl;
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] Shader 编译成功" << std::endl;

    // 创建 RenderPass
    Diligent::RenderPassAttachmentDesc RTAtt{};
    RTAtt.Format = swapChain->GetDesc().ColorBufferFormat;
    RTAtt.LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    RTAtt.StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    RTAtt.StencilLoadOp = Diligent::ATTACHMENT_LOAD_OP_DISCARD;
    RTAtt.StencilStoreOp = Diligent::ATTACHMENT_STORE_OP_DISCARD;
    RTAtt.InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
    RTAtt.FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;

    Diligent::AttachmentReference RTRef{0, Diligent::RESOURCE_STATE_RENDER_TARGET};
    Diligent::SubpassDesc subpass{};
    subpass.RenderTargetAttachmentCount = 1;
    subpass.pRenderTargetAttachments = &RTRef;

    Diligent::RenderPassDesc rpDesc{};
    rpDesc.Name = "TriangleRP";
    rpDesc.AttachmentCount = 1;
    rpDesc.pAttachments = &RTAtt;
    rpDesc.SubpassCount = 1;
    rpDesc.pSubpasses = &subpass;

    Diligent::RefCntAutoPtr<Diligent::IRenderPass> renderPass;
    device->CreateRenderPass(rpDesc, &renderPass);

    if (!renderPass) {
        std::cerr << "[FAIL] RenderPass 创建失败!" << std::endl;
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] RenderPass 创建成功" << std::endl;

    // 创建 PSO
    Diligent::GraphicsPipelineStateCreateInfo psoCI;
    psoCI.PSODesc.Name = "TrianglePSO";
    psoCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    psoCI.GraphicsPipeline.NumRenderTargets = 0;
    psoCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_UNKNOWN;
    psoCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
    psoCI.GraphicsPipeline.pRenderPass = renderPass;
    psoCI.GraphicsPipeline.SubpassIndex = 0;
    psoCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    psoCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

    psoCI.pVS = vs;
    psoCI.pPS = ps;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso;
    device->CreateGraphicsPipelineState(psoCI, &pso);

    if (!pso) {
        std::cerr << "[FAIL] PSO 创建失败!" << std::endl;
        swapChain->Release();
        context->Release();
        device->Release();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::cout << "[OK] PSO 创建成功" << std::endl;

    // 渲染循环
    std::cout << "\n[INFO] 开始渲染循环... 按 ESC 退出" << std::endl;

    bool running = true;
    int frameCount = 0;
    auto startTime = SDL_GetTicks();

    while (running) {
        // 处理事件
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT) running = false;
            if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_ESCAPE) running = false;
            if (evt.type == SDL_EVENT_WINDOW_RESIZED) {
                int w = evt.window.data1, h = evt.window.data2;
                if (w > 0 && h > 0) {
                    swapChain->Resize(static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
                }
            }
        }

        // 获取当前 Back Buffer RTV
        Diligent::ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();

        // 创建 Framebuffer
        Diligent::FramebufferDesc fbDesc{};
        fbDesc.pRenderPass = renderPass;
        fbDesc.AttachmentCount = 1;
        fbDesc.ppAttachments = &pRTV;

        Diligent::RefCntAutoPtr<Diligent::IFramebuffer> framebuffer;
        device->CreateFramebuffer(fbDesc, &framebuffer);

        // Begin Render Pass
        Diligent::OptimizedClearValue clearVal{};
        clearVal.Color[0] = 0.1f;
        clearVal.Color[1] = 0.1f;
        clearVal.Color[2] = 0.1f;
        clearVal.Color[3] = 1.0f;

        Diligent::BeginRenderPassAttribs beginRP{};
        beginRP.pRenderPass = renderPass;
        beginRP.pFramebuffer = framebuffer;
        beginRP.ClearValueCount = 1;
        beginRP.pClearValues = &clearVal;
        beginRP.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        context->BeginRenderPass(beginRP);
        context->SetPipelineState(pso);

        // Draw
        Diligent::DrawAttribs da;
        da.NumVertices = 3;
        da.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        context->Draw(da);

        context->EndRenderPass();

        // Present
        swapChain->Present(0);

        frameCount++;
    }

    // 统计
    auto endTime = SDL_GetTicks();
    float elapsed = (endTime - startTime) / 1000.0f;
    float fps = frameCount / elapsed;

    std::cout << "\n============================================" << std::endl;
    std::cout << "  [PASS] Phase 1 Smoke Test 通过!" << std::endl;
    std::cout << "  帧数: " << frameCount << std::endl;
    std::cout << "  时间: " << elapsed << "s" << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "============================================" << std::endl;

    // 清理
    swapChain->Release();
    context->Release();
    device->Release();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
