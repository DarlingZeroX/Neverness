// TestResourceCreation.cpp — Phase 2 test: Buffer, Texture, Sampler, RenderTarget creation
// Tests resource creation through the NNRuntimeRender interface layer

#include <NNRuntimeRenderBootstrap/Include/NNRenderBootstrap.h>
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeRender/Resources/INNTexture.h>
#include <NNRuntimeRender/Resources/INNSampler.h>
#include <NNRuntimeRender/RenderTarget/INNRenderTarget.h>
#include <NNRuntimeCore/NNObject.h>
#include <NNRuntimeCore/Handle/NNObjectHandleRegistry.h>
#include <NNRuntimeCore/Handle/NNHandleTypes.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <cassert>

using namespace NN::Runtime::Core;
using namespace NN::Runtime::Render;

static int g_TestsPassed = 0;
static int g_TestsFailed = 0;

#define TEST(name) \
    std::cout << "  [TEST] " << name << "... ";

#define PASS() \
    std::cout << "PASS" << std::endl; \
    g_TestsPassed++;

#define FAIL(reason) \
    std::cout << "FAIL: " << reason << std::endl; \
    g_TestsFailed++;

int main(int argc, char* argv[])
{
    std::cout << "=== NNRuntime Phase 2: Resource Creation Tests ===" << std::endl;

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window (hidden for resource tests)
    SDL_Window* window = SDL_CreateWindow("Phase2Test", 800, 600, SDL_WINDOW_HIDDEN);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create render device
    NNRenderDeviceCreateInfo createInfo{};
    createInfo.Backend = NNRenderBackendType::Auto;
    createInfo.Window = window;
    createInfo.Width = 800;
    createInfo.Height = 600;
    createInfo.EnableValidation = true;

    NNRef<INNRenderDevice> device = NNRenderBootstrap::CreateDevice(createInfo);
    if (!device)
    {
        std::cerr << "Failed to create render device!" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Device created: " << device->GetDeviceInfo().DeviceName << std::endl;
    std::cout << std::endl;

    // ====== Test 1: Create Vertex Buffer ======
    {
        TEST("Create Vertex Buffer (Static VB)")

        float vertices[] = {
            0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
           -0.5f,-0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
            0.5f,-0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        };

        NNBufferDesc desc{};
        desc.Type = NNBufferType::Vertex;
        desc.Usage = NNBufferUsage::Static;
        desc.Size = sizeof(vertices);
        desc.Stride = 7 * sizeof(float);

        NNRef<INNBuffer> vb = device->CreateBuffer(desc, vertices);
        if (!vb)
        {
            FAIL("CreateBuffer returned null");
        }
        else
        {
            assert(vb->GetSize() == sizeof(vertices));
            assert(vb->GetDesc().Type == NNBufferType::Vertex);
            PASS();
        }
    }

    // ====== Test 2: Create Index Buffer ======
    {
        TEST("Create Index Buffer (Static IB)")

        uint16_t indices[] = { 0, 1, 2 };

        NNBufferDesc desc{};
        desc.Type = NNBufferType::Index;
        desc.Usage = NNBufferUsage::Static;
        desc.Size = sizeof(indices);

        NNRef<INNBuffer> ib = device->CreateBuffer(desc, indices);
        if (!ib)
        {
            FAIL("CreateBuffer returned null");
        }
        else
        {
            assert(ib->GetSize() == sizeof(indices));
            assert(ib->GetDesc().Type == NNBufferType::Index);
            PASS();
        }
    }

    // ====== Test 3: Create Constant Buffer ======
    {
        TEST("Create Constant Buffer (Dynamic CB)")

        NNBufferDesc desc{};
        desc.Type = NNBufferType::Constant;
        desc.Usage = NNBufferUsage::Dynamic;
        desc.Size = 16; // float4
        desc.CPUAccessible = true;

        // Dynamic buffers must NOT have initial data at creation
        NNRef<INNBuffer> cb = device->CreateBuffer(desc);
        if (!cb)
        {
            FAIL("CreateBuffer returned null");
        }
        else
        {
            // Constant buffers get 16-byte aligned, so size may be rounded up
            assert(cb->GetSize() >= 16);
            assert(cb->GetDesc().Type == NNBufferType::Constant);
            PASS();
        }
    }

    // ====== Test 4: Create Texture 2D ======
    {
        TEST("Create Texture 2D (1024x1024 RGBA8)")

        NNTextureDesc desc{};
        desc.Width = 1024;
        desc.Height = 1024;
        desc.Format = NNPixelFormat::RGBA8_UNORM;
        desc.Dimension = NNTextureDimension::Tex2D;
        desc.Usage = NNTextureUsage::Default;
        desc.DebugName = "TestTexture2D";

        NNRef<INNTexture> tex = device->CreateTexture(desc);
        if (!tex)
        {
            FAIL("CreateTexture returned null");
        }
        else
        {
            assert(tex->GetWidth() == 1024);
            assert(tex->GetHeight() == 1024);
            assert(tex->GetDesc().Format == NNPixelFormat::RGBA8_UNORM);
            PASS();
        }
    }

    // ====== Test 5: Create Texture for Render Target ======
    {
        TEST("Create Texture (RenderTarget usage)")

        NNTextureDesc desc{};
        desc.Width = 512;
        desc.Height = 512;
        desc.Format = NNPixelFormat::RGBA8_UNORM;
        desc.Usage = NNTextureUsage::RenderTarget;
        desc.DebugName = "TestRTTexture";

        NNRef<INNTexture> tex = device->CreateTexture(desc);
        if (!tex)
        {
            FAIL("CreateTexture returned null");
        }
        else
        {
            assert(tex->GetWidth() == 512);
            assert(tex->GetHeight() == 512);
            PASS();
        }
    }

    // ====== Test 6: Create Depth Texture ======
    {
        TEST("Create Depth Texture (D32_FLOAT)")

        NNTextureDesc desc{};
        desc.Width = 800;
        desc.Height = 600;
        desc.Format = NNPixelFormat::D32_FLOAT;
        desc.Usage = NNTextureUsage::DepthStencil;
        desc.DebugName = "TestDepthTex";

        NNRef<INNTexture> tex = device->CreateTexture(desc);
        if (!tex)
        {
            FAIL("CreateTexture returned null");
        }
        else
        {
            assert(tex->GetWidth() == 800);
            PASS();
        }
    }

    // ====== Test 7: Create Sampler ======
    {
        TEST("Create Sampler (Linear/Anisotropic)")

        NNSamplerDesc desc{};
        desc.MinFilter = NNFilterMode::Linear;
        desc.MagFilter = NNFilterMode::Linear;
        desc.MipFilter = NNFilterMode::Linear;
        desc.AddressU = NNAddressMode::Wrap;
        desc.AddressV = NNAddressMode::Wrap;
        desc.MaxAnisotropy = 16;

        NNRef<INNSampler> sampler = device->CreateSampler(desc);
        if (!sampler)
        {
            FAIL("CreateSampler returned null");
        }
        else
        {
            assert(sampler->GetDesc().MaxAnisotropy == 16);
            PASS();
        }
    }

    // ====== Test 8: Create Render Target ======
    {
        TEST("Create Render Target (Color+Depth, 800x600)")

        NNRenderTargetDesc desc{};
        desc.Width = 800;
        desc.Height = 600;
        desc.ColorFormat = NNPixelFormat::RGBA8_UNORM;
        desc.DepthFormat = NNPixelFormat::D32_FLOAT;
        desc.ColorAttachmentCount = 1;
        desc.DebugName = "TestRT";

        NNRef<INNRenderTarget> rt = device->CreateRenderTarget(desc);
        if (!rt)
        {
            FAIL("CreateRenderTarget returned null");
        }
        else
        {
            assert(rt->GetWidth() == 800);
            assert(rt->GetHeight() == 600);
            PASS();
        }
    }

    // ====== Test 9: HandleRegistry Integration ======
    {
        TEST("HandleRegistry: Register and retrieve Buffer")

        NNObjectHandleRegistry registry;

        float vertices[] = { 0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };

        NNBufferDesc desc{};
        desc.Type = NNBufferType::Vertex;
        desc.Usage = NNBufferUsage::Static;
        desc.Size = sizeof(vertices);

        NNRef<INNBuffer> buf = device->CreateBuffer(desc, vertices);
        if (!buf)
        {
            FAIL("CreateBuffer returned null");
        }
        else
        {
            NNRenderHandle handle = registry.Register(NNHandleType::Buffer, buf);
            assert(handle != NN_INVALID_HANDLE);

            INNObject* obj = registry.Get(handle);
            assert(obj != nullptr);

            // Verify it's the same buffer
            auto* bufBack = dynamic_cast<INNBuffer*>(obj);
            assert(bufBack != nullptr);
            assert(bufBack->GetSize() == sizeof(vertices));

            registry.Release(handle);
            PASS();
        }
    }

    // ====== Test 10: HandleRegistry Type Safety ======
    {
        TEST("HandleRegistry: Type-safe retrieval")

        NNObjectHandleRegistry registry;

        NNSamplerDesc sDesc{};
        NNRef<INNSampler> sampler = device->CreateSampler(sDesc);
        if (!sampler)
        {
            FAIL("CreateSampler returned null");
        }
        else
        {
            NNRenderHandle handle = registry.Register(NNHandleType::Sampler, sampler);
            assert(handle != NN_INVALID_HANDLE);
            assert(GetHandleType(handle) == NNHandleType::Sampler);

            auto* samplerBack = registry.GetAs<INNSampler>(handle);
            assert(samplerBack != nullptr);
            assert(samplerBack == sampler.Get());

            registry.Release(handle);
            PASS();
        }
    }

    // ====== Summary ======
    std::cout << std::endl;
    std::cout << "=== Results: " << g_TestsPassed << " passed, "
              << g_TestsFailed << " failed ===" << std::endl;

    device = nullptr;
    SDL_DestroyWindow(window);
    SDL_Quit();

    return g_TestsFailed > 0 ? 1 : 0;
}
