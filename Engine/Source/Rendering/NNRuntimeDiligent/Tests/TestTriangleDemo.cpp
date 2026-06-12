// TestTriangleDemo.cpp - Phase 1: Diligent Triangle (Vulkan RenderPass + Framebuffer)
#include <iostream>
#include <SDL3/SDL.h>
#include <NNRuntimeCore/NNObject.h>
#include <NNRuntimeCore/Handle/NNObjectHandleRegistry.h>
#include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
#include <NNRuntimeDiligent/Command/NNDiligentCommandList.h>
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypes.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/RenderPass.h"
#include "Graphics/GraphicsEngine/interface/Framebuffer.h"

namespace nnr = NN::Runtime::Render;
namespace nnc = NN::Runtime::Core;

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

int main()
{
    std::cout << "=== NNRuntimeDiligent Phase 1 ===" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cerr << "SDL FAILED: " << SDL_GetError() << std::endl; return 1; }
    SDL_Window* sdlWnd = SDL_CreateWindow("Diligent Triangle", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!sdlWnd) { SDL_Quit(); return 1; }

    nnr::NNRenderDeviceCreateInfo devCI{};
    devCI.Window = sdlWnd; devCI.Width = 1280; devCI.Height = 720;
    devCI.EnableValidation = true; devCI.Backend = nnr::NNRenderBackendType::Auto;

    auto* dilDevice = new NNDiligent::NNDiligentDevice();
    dilDevice->AddRef();
    if (!dilDevice->Initialize(devCI)) { dilDevice->Release(); SDL_DestroyWindow(sdlWnd); SDL_Quit(); return 1; }
    std::cout << "Backend: " << dilDevice->GetDeviceInfo().DeviceName << std::endl;

    auto* dev = dilDevice->GetDiligentDevice();
    auto* ctx = dilDevice->GetDiligentContext();
    auto* sc = dilDevice->GetDiligentSwapChain();

    // === RenderPass ===
    ::Diligent::RenderPassAttachmentDesc RTAtt{};
    RTAtt.Format = sc->GetDesc().ColorBufferFormat;
    RTAtt.LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    RTAtt.StoreOp = ::Diligent::ATTACHMENT_STORE_OP_STORE;
    RTAtt.StencilLoadOp = ::Diligent::ATTACHMENT_LOAD_OP_DISCARD;
    RTAtt.StencilStoreOp = ::Diligent::ATTACHMENT_STORE_OP_DISCARD;
    RTAtt.InitialState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
    RTAtt.FinalState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;

    ::Diligent::AttachmentReference RTRef{0, ::Diligent::RESOURCE_STATE_RENDER_TARGET};
    ::Diligent::SubpassDesc subpass{};
    subpass.RenderTargetAttachmentCount = 1;
    subpass.pRenderTargetAttachments = &RTRef;

    ::Diligent::RenderPassDesc rpDesc{};
    rpDesc.Name = "TriangleRP";
    rpDesc.AttachmentCount = 1;
    rpDesc.pAttachments = &RTAtt;
    rpDesc.SubpassCount = 1;
    rpDesc.pSubpasses = &subpass;

    ::Diligent::RefCntAutoPtr<::Diligent::IRenderPass> renderPass;
    dev->CreateRenderPass(rpDesc, &renderPass);
    if (!renderPass) { std::cerr << "RenderPass FAILED!" << std::endl; return 1; }
    std::cout << "RenderPass OK" << std::endl;

    // === Shaders ===
    ::Diligent::ShaderCreateInfo sci;
    sci.SourceLanguage = ::Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    sci.Desc.UseCombinedTextureSamplers = true;

    ::Diligent::RefCntAutoPtr<::Diligent::IShader> myVS;
    sci.Desc.ShaderType = ::Diligent::SHADER_TYPE_VERTEX;
    sci.EntryPoint = "main"; sci.Desc.Name = "VS"; sci.Source = gTriangleVS;
    dev->CreateShader(sci, &myVS);

    ::Diligent::RefCntAutoPtr<::Diligent::IShader> myPS;
    sci.Desc.ShaderType = ::Diligent::SHADER_TYPE_PIXEL;
    sci.Desc.Name = "PS"; sci.Source = gTrianglePS;
    dev->CreateShader(sci, &myPS);

    // === PSO ===
    ::Diligent::GraphicsPipelineStateCreateInfo psoCI;
    psoCI.PSODesc.Name = "TrianglePSO";
    psoCI.PSODesc.PipelineType = ::Diligent::PIPELINE_TYPE_GRAPHICS;
    psoCI.GraphicsPipeline.NumRenderTargets = 0;
    psoCI.GraphicsPipeline.RTVFormats[0] = ::Diligent::TEX_FORMAT_UNKNOWN;
    psoCI.GraphicsPipeline.DSVFormat = ::Diligent::TEX_FORMAT_UNKNOWN;
    psoCI.GraphicsPipeline.pRenderPass = renderPass;
    psoCI.GraphicsPipeline.SubpassIndex = 0;
    psoCI.GraphicsPipeline.PrimitiveTopology = ::Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    psoCI.GraphicsPipeline.RasterizerDesc.CullMode = ::Diligent::CULL_MODE_NONE;
    psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
    psoCI.pVS = myVS;
    psoCI.pPS = myPS;

    ::Diligent::RefCntAutoPtr<::Diligent::IPipelineState> myPSO;
    dev->CreateGraphicsPipelineState(psoCI, &myPSO);

    if (!myVS || !myPS || !myPSO) { std::cerr << "PSO FAILED!" << std::endl; return 1; }
    std::cout << "Shaders + PSO OK" << std::endl;

    // Handle test
    nnc::NNObjectHandleRegistry hReg;
    auto hDev = hReg.Register(nnc::NNHandleType::Buffer, nnc::NNRef<nnc::INNObject>(dilDevice));
    std::cout << "Handle: " << (hReg.Get(hDev) ? "OK" : "FAIL") << std::endl;
    hReg.Release(hDev);

    // === Render Loop ===
    std::cout << "Rendering... ESC to exit" << std::endl;

    bool running = true;
    int frameCount = 0;
    auto startTime = SDL_GetTicks();

    while (running)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_EVENT_QUIT) running = false;
            if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_ESCAPE) running = false;
            if (evt.type == SDL_EVENT_WINDOW_RESIZED)
            {
                int w = evt.window.data1, h = evt.window.data2;
                if (w > 0 && h > 0) sc->Resize((uint32_t)w, (uint32_t)h);
            }
        }

        // 获取当前 back buffer RTV，创建 Framebuffer
        ::Diligent::ITextureView* pRTV = sc->GetCurrentBackBufferRTV();

        ::Diligent::FramebufferDesc fbDesc{};
        fbDesc.pRenderPass = renderPass;
        fbDesc.AttachmentCount = 1;
        fbDesc.ppAttachments = &pRTV;

        ::Diligent::RefCntAutoPtr<::Diligent::IFramebuffer> framebuffer;
        dev->CreateFramebuffer(fbDesc, &framebuffer);

        // Begin Render Pass
        ::Diligent::OptimizedClearValue clearVal{};
        clearVal.Color[0] = 0.35f;
        clearVal.Color[1] = 0.35f;
        clearVal.Color[2] = 0.35f;
        clearVal.Color[3] = 1.0f;

        ::Diligent::BeginRenderPassAttribs beginRP{};
        beginRP.pRenderPass = renderPass;
        beginRP.pFramebuffer = framebuffer;
        beginRP.ClearValueCount = 1;
        beginRP.pClearValues = &clearVal;
        beginRP.StateTransitionMode = ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        ctx->BeginRenderPass(beginRP);
        ctx->SetPipelineState(myPSO);

        ::Diligent::DrawAttribs da;
        da.NumVertices = 3;
        ctx->Draw(da);

        ctx->EndRenderPass();

        sc->Present();
        frameCount++;
    }

    float elapsed = (SDL_GetTicks() - startTime) / 1000.0f;
    std::cout << frameCount << " frames, " << elapsed << "s, " << (frameCount / elapsed) << " FPS" << std::endl;

    myPSO = nullptr; myVS = nullptr; myPS = nullptr; renderPass = nullptr;
    dilDevice->Release();
    SDL_DestroyWindow(sdlWnd);
    SDL_Quit();
    std::cout << "=== Phase 1 Complete ===" << std::endl;
    return 0;
}
