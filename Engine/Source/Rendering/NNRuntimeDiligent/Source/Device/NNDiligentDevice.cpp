// NNDiligentDevice.cpp 鈥?瀹屽叏闄愬畾鍚嶏紝涓嶇敤 using

#include "../../Device/NNDiligentDevice.h"
#include "../../Command/NNDiligentCommandList.h"
#include "../../Resources/NNDiligentBuffer.h"
#include "../../Resources/NNDiligentTexture.h"
#include "../../Resources/NNDiligentSampler.h"
#include "../../Resources/NNDiligentRenderTarget.h"
#include "../../Pipeline/NNDiligentShader.h"
#include "../../Pipeline/NNDiligentPipelineState.h"
#include <iostream>
#include <vector>
#include <SDL3/SDL.h>

#include "GraphicsAccessories.hpp"

namespace NNDiligent
{
    NNDiligentDevice::NNDiligentDevice() {}
    NNDiligentDevice::~NNDiligentDevice() { Shutdown(); }

    bool NNDiligentDevice::Initialize(const NN::Runtime::Render::NNRenderDeviceCreateInfo& info)
    {
        std::cout << "[NNDiligentDevice] Initializing..." << std::endl;

        SDL_Window* window = static_cast<SDL_Window*>(info.Window);
        if (!window) return false;

        ::Diligent::NativeWindow nw{};
#if PLATFORM_WIN32
        SDL_PropertiesID props = SDL_GetWindowProperties(window);
        nw.hWnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (!nw.hWnd) return false;
#endif

        ::Diligent::SwapChainDesc scDesc;
        scDesc.Width  = info.Width;
        scDesc.Height = info.Height;

        auto backend = info.Backend;
        if (backend == NN::Runtime::Render::NNRenderBackendType::Auto)
        {
#if VULKAN_SUPPORTED
            backend = NN::Runtime::Render::NNRenderBackendType::Backend_Vulkan;
#elif D3D12_SUPPORTED
            backend = NN::Runtime::Render::NNRenderBackendType::Backend_D3D12;
#elif D3D11_SUPPORTED
            backend = NN::Runtime::Render::NNRenderBackendType::Backend_D3D11;
#elif GL_SUPPORTED
            backend = NN::Runtime::Render::NNRenderBackendType::Backend_OpenGL;
#else
            return false;
#endif
        }

        bool ok = false;

#if VULKAN_SUPPORTED
        if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_Vulkan)
        {
            std::cout << "[NNDiligentDevice] Vulkan" << std::endl;
            auto* F = ::Diligent::LoadAndGetEngineFactoryVk();
            if (F) {
                ::Diligent::EngineVkCreateInfo ci;
                ci.SetValidationLevel(info.EnableValidation ? ::Diligent::VALIDATION_LEVEL_2 : ::Diligent::VALIDATION_LEVEL_DISABLED);
                F->CreateDeviceAndContextsVk(ci, &m_Device, &m_Context);
                if (m_Device && m_Context) { F->CreateSwapChainVk(m_Device, m_Context, scDesc, nw, &m_SwapChain); ok = true; }
            }
        }
#endif
#if D3D12_SUPPORTED
        if (!ok && backend == NN::Runtime::Render::NNRenderBackendType::Backend_D3D12)
        {
            std::cout << "[NNDiligentDevice] D3D12" << std::endl;
            auto* F = ::Diligent::LoadAndGetEngineFactoryD3D12();
            if (F) {
                ::Diligent::EngineD3D12CreateInfo ci;
                ci.SetValidationLevel(info.EnableValidation ? ::Diligent::VALIDATION_LEVEL_2 : ::Diligent::VALIDATION_LEVEL_DISABLED);
                // 增大 GPU 描述符堆（RmlDiligent 20+ PSO × SRB + Renderer2D SRB 缓存）
                ci.GPUDescriptorHeapSize[0]        = 32768;  // CBV/SRV/UAV 静态堆（默认 16384）
                ci.GPUDescriptorHeapDynamicSize[0] = 16384;  // CBV/SRV/UAV 动态堆（默认 8192）
                // sampler 堆：默认 1024+1024=2048（D3D12 上限）
                // RmlDiligent SRB 缓存 + Renderer2D SRB 缓存 需要更多空间
                ci.GPUDescriptorHeapSize[1]        = 1536;   // sampler 静态堆（默认 1024）
                ci.GPUDescriptorHeapDynamicSize[1] = 512;    // sampler 动态堆（默认 1024）
                F->CreateDeviceAndContextsD3D12(ci, &m_Device, &m_Context);
                if (m_Device && m_Context) { F->CreateSwapChainD3D12(m_Device, m_Context, scDesc, ::Diligent::FullScreenModeDesc{}, nw, &m_SwapChain); ok = true; }
            }
        }
#endif
#if D3D11_SUPPORTED
        if (!ok && backend == NN::Runtime::Render::NNRenderBackendType::Backend_D3D11)
        {
            std::cout << "[NNDiligentDevice] D3D11" << std::endl;
            auto* F = ::Diligent::LoadAndGetEngineFactoryD3D11();
            if (F) {
                ::Diligent::EngineD3D11CreateInfo ci;
                ci.SetValidationLevel(info.EnableValidation ? ::Diligent::VALIDATION_LEVEL_2 : ::Diligent::VALIDATION_LEVEL_DISABLED);
                F->CreateDeviceAndContextsD3D11(ci, &m_Device, &m_Context);
                if (m_Device && m_Context) { F->CreateSwapChainD3D11(m_Device, m_Context, scDesc, ::Diligent::FullScreenModeDesc{}, nw, &m_SwapChain); ok = true; }
            }
        }
#endif
#if GL_SUPPORTED
        if (!ok && backend == NN::Runtime::Render::NNRenderBackendType::Backend_OpenGL)
        {
            std::cout << "[NNDiligentDevice] OpenGL" << std::endl;
            auto* F = ::Diligent::LoadAndGetEngineFactoryOpenGL();
            if (F) {
                ::Diligent::EngineGLCreateInfo ci;
                ci.Window = nw;
                ci.SetValidationLevel(info.EnableValidation ? ::Diligent::VALIDATION_LEVEL_2 : ::Diligent::VALIDATION_LEVEL_DISABLED);
                F->CreateDeviceAndSwapChainGL(ci, &m_Device, &m_Context, scDesc, &m_SwapChain);
                if (m_Device && m_Context && m_SwapChain) ok = true;
            }
        }
#endif

        if (!ok || !m_Device || !m_Context || !m_SwapChain) { Shutdown(); return false; }

        m_DeviceInfo.Backend = backend;
        m_DeviceInfo.MaxTextureSize = m_Device->GetAdapterInfo().Texture.MaxTexture2DDimension;
        m_DeviceInfo.MaxBufferSize = 256ULL * 1024 * 1024;
        m_DeviceInfo.SupportsCompute = m_Device->GetDeviceInfo().Features.ComputeShaders != 0;
        m_DeviceInfo.SupportsRayTracing = m_Device->GetDeviceInfo().Features.RayTracing != 0;

        const char* name = "Unknown";
        if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_Vulkan) name = "Vulkan";
        else if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_D3D12) name = "D3D12";
        else if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_D3D11) name = "D3D11";
        else if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_OpenGL) name = "OpenGL";
        else if (backend == NN::Runtime::Render::NNRenderBackendType::Backend_Metal) name = "Metal";
        m_DeviceInfo.DeviceName = name;

        std::cout << "[NNDiligentDevice] OK! Backend: " << m_DeviceInfo.DeviceName << std::endl;
        return true;
    }

    void NNDiligentDevice::Shutdown()
    {
        m_ImmediateCmd = nullptr;
        if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
        if (m_Context)   { m_Context->Release();   m_Context = nullptr; }
        if (m_Device)    { m_Device->Release();    m_Device = nullptr; }
    }

    // ===== Helper: Convert NNPixelFormat to Diligent TEXTURE_FORMAT =====
    static ::Diligent::TEXTURE_FORMAT ToDiligentFormat(NN::Runtime::Render::NNPixelFormat fmt)
    {
        using namespace NN::Runtime::Render;
        switch (fmt)
        {
            case NNPixelFormat::RGBA8_UNORM:        return ::Diligent::TEX_FORMAT_RGBA8_UNORM;
            case NNPixelFormat::RGBA8_SRGB:         return ::Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            case NNPixelFormat::BGRA8_UNORM:        return ::Diligent::TEX_FORMAT_BGRA8_UNORM;
            case NNPixelFormat::R32_FLOAT:          return ::Diligent::TEX_FORMAT_R32_FLOAT;
            case NNPixelFormat::RG32_FLOAT:         return ::Diligent::TEX_FORMAT_RG32_FLOAT;
            case NNPixelFormat::RGBA32_FLOAT:       return ::Diligent::TEX_FORMAT_RGBA32_FLOAT;
            case NNPixelFormat::D32_FLOAT:          return ::Diligent::TEX_FORMAT_D32_FLOAT;
            case NNPixelFormat::D24_UNORM_S8_UINT:  return ::Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            case NNPixelFormat::BC1_UNORM:          return ::Diligent::TEX_FORMAT_BC1_UNORM;
            case NNPixelFormat::BC3_UNORM:          return ::Diligent::TEX_FORMAT_BC3_UNORM;
            case NNPixelFormat::BC5_UNORM:          return ::Diligent::TEX_FORMAT_BC5_UNORM;
            case NNPixelFormat::BC7_UNORM:          return ::Diligent::TEX_FORMAT_BC7_UNORM;
            default:                                return ::Diligent::TEX_FORMAT_UNKNOWN;
        }
    }

    // ===== CreateBuffer =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNBuffer> NNDiligentDevice::CreateBuffer(
        const NN::Runtime::Render::NNBufferDesc& desc, const void* initialData)
    {
        if (!m_Device) return {};

        ::Diligent::BufferDesc bufDesc;
        bufDesc.Name = "NNBuffer";
        bufDesc.Size = desc.Size;

        // Map buffer type to bind flags
        switch (desc.Type)
        {
            case NN::Runtime::Render::NNBufferType::Vertex:
                bufDesc.BindFlags = ::Diligent::BIND_VERTEX_BUFFER;
                break;
            case NN::Runtime::Render::NNBufferType::Index:
                bufDesc.BindFlags = ::Diligent::BIND_INDEX_BUFFER;
                break;
            case NN::Runtime::Render::NNBufferType::Constant:
                bufDesc.BindFlags = ::Diligent::BIND_UNIFORM_BUFFER;
                // Constant buffers must be 16-byte aligned
                bufDesc.Size = (desc.Size + 15u) & ~15u;
                break;
            case NN::Runtime::Render::NNBufferType::Storage:
                bufDesc.BindFlags = ::Diligent::BIND_SHADER_RESOURCE;
                bufDesc.Mode = ::Diligent::BUFFER_MODE_STRUCTURED;
                bufDesc.ElementByteStride = desc.Stride > 0 ? desc.Stride : 16;
                break;
        }

        // Map usage
        switch (desc.Usage)
        {
            case NN::Runtime::Render::NNBufferUsage::Static:
                bufDesc.Usage = ::Diligent::USAGE_IMMUTABLE;
                break;
            case NN::Runtime::Render::NNBufferUsage::Dynamic:
                bufDesc.Usage = ::Diligent::USAGE_DYNAMIC;
                bufDesc.CPUAccessFlags = ::Diligent::CPU_ACCESS_WRITE;
                break;
            case NN::Runtime::Render::NNBufferUsage::Staging:
                bufDesc.Usage = ::Diligent::USAGE_STAGING;
                bufDesc.CPUAccessFlags = ::Diligent::CPU_ACCESS_READ;
                break;
        }

        if (desc.CPUAccessible && desc.Usage == NN::Runtime::Render::NNBufferUsage::Dynamic)
        {
            bufDesc.CPUAccessFlags = ::Diligent::CPU_ACCESS_WRITE;
        }

        // Initial data — immutable buffers MUST have initial data at creation;
        // dynamic buffers must NOT have initial data at creation (use Map/UpdateBuffer later).
        ::Diligent::BufferData initData;
        const ::Diligent::BufferData* pInitData = nullptr;
        if (initialData && desc.Size > 0 && desc.Usage != NN::Runtime::Render::NNBufferUsage::Dynamic)
        {
            initData.pData = initialData;
            initData.DataSize = desc.Size;
            pInitData = &initData;
        }

        ::Diligent::IBuffer* buffer = nullptr;
        m_Device->CreateBuffer(bufDesc, pInitData, &buffer);
        if (!buffer) return {};

        auto* wrapper = new NNDiligentBuffer(buffer, desc);
        wrapper->SetDeviceContext(m_Context);
        wrapper->AddRef();
        buffer->Release();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNBuffer>(wrapper);
    }

    // ===== CreateTexture =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNTexture> NNDiligentDevice::CreateTexture(
        const NN::Runtime::Render::NNTextureDesc& desc, const void* initialData)
    {
        if (!m_Device) return {};

        ::Diligent::TextureDesc texDesc;
        texDesc.Name = desc.DebugName ? desc.DebugName : "NNTexture";

        // Map dimension
        switch (desc.Dimension)
        {
            case NN::Runtime::Render::NNTextureDimension::Tex1D:
                texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_1D;
                break;
            case NN::Runtime::Render::NNTextureDimension::Tex2D:
                texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_2D;
                break;
            case NN::Runtime::Render::NNTextureDimension::Tex3D:
                texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_3D;
                break;
            case NN::Runtime::Render::NNTextureDimension::TexCube:
                texDesc.Type = ::Diligent::RESOURCE_DIM_TEX_CUBE;
                break;
        }

        texDesc.Width  = desc.Width;
        texDesc.Height = desc.Height;
        texDesc.Depth  = desc.Depth;
        texDesc.MipLevels = desc.MipLevels;
        texDesc.ArraySize = desc.ArraySize;
        texDesc.Format = ToDiligentFormat(desc.Format);
        texDesc.SampleCount = desc.SampleCount;

        // Map usage
        switch (desc.Usage)
        {
            case NN::Runtime::Render::NNTextureUsage::Default:
                texDesc.Usage = ::Diligent::USAGE_DEFAULT;
                texDesc.BindFlags = ::Diligent::BIND_SHADER_RESOURCE;
                break;
            case NN::Runtime::Render::NNTextureUsage::RenderTarget:
                texDesc.Usage = ::Diligent::USAGE_DEFAULT;
                texDesc.BindFlags = ::Diligent::BIND_RENDER_TARGET | ::Diligent::BIND_SHADER_RESOURCE;
                break;
            case NN::Runtime::Render::NNTextureUsage::DepthStencil:
                texDesc.Usage = ::Diligent::USAGE_DEFAULT;
                texDesc.BindFlags = ::Diligent::BIND_DEPTH_STENCIL;
                break;
            case NN::Runtime::Render::NNTextureUsage::Staging:
                texDesc.Usage = ::Diligent::USAGE_STAGING;
                texDesc.CPUAccessFlags = ::Diligent::CPU_ACCESS_READ;
                break;
        }

        ::Diligent::ITexture* texture = nullptr;

        if (initialData)
        {
            // 计算像素行 stride（字节）
            auto bpp = ::Diligent::GetTextureFormatAttribs(texDesc.Format).GetElementSize();
            ::Diligent::TextureSubResData mip0;
            mip0.pData  = initialData;
            mip0.Stride = static_cast<::Diligent::Uint64>(desc.Width) * bpp;

            ::Diligent::TextureData texData;
            texData.pSubResources   = &mip0;
            texData.NumSubresources = 1;
            m_Device->CreateTexture(texDesc, &texData, &texture);
        }
        else
        {
            m_Device->CreateTexture(texDesc, nullptr, &texture);
        }

        if (!texture) return {};

        // 初始数据上传后纹理处于 COPY_DEST 状态，需要转为 SHADER_RESOURCE
        if (initialData && texDesc.BindFlags & ::Diligent::BIND_SHADER_RESOURCE)
        {
            ::Diligent::StateTransitionDesc transition;
            transition.pResource = texture;
            transition.OldState  = ::Diligent::RESOURCE_STATE_COPY_DEST;
            transition.NewState  = ::Diligent::RESOURCE_STATE_SHADER_RESOURCE;
            transition.Flags     = ::Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE;
            m_Context->TransitionResourceStates(1, &transition);
        }

        auto* wrapper = new NNDiligentTexture(texture, desc);
        wrapper->AddRef();
        texture->Release();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNTexture>(wrapper);
    }

    // ===== CreateSampler =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNSampler> NNDiligentDevice::CreateSampler(
        const NN::Runtime::Render::NNSamplerDesc& desc)
    {
        if (!m_Device) return {};

        auto ToDiligentFilter = [](NN::Runtime::Render::NNFilterMode mode) -> ::Diligent::FILTER_TYPE
        {
            switch (mode)
            {
                case NN::Runtime::Render::NNFilterMode::Point:       return ::Diligent::FILTER_TYPE_POINT;
                case NN::Runtime::Render::NNFilterMode::Linear:      return ::Diligent::FILTER_TYPE_LINEAR;
                case NN::Runtime::Render::NNFilterMode::Anisotropic: return ::Diligent::FILTER_TYPE_ANISOTROPIC;
                default: return ::Diligent::FILTER_TYPE_LINEAR;
            }
        };

        auto ToDiligentAddress = [](NN::Runtime::Render::NNAddressMode mode) -> ::Diligent::TEXTURE_ADDRESS_MODE
        {
            switch (mode)
            {
                case NN::Runtime::Render::NNAddressMode::Wrap:   return ::Diligent::TEXTURE_ADDRESS_WRAP;
                case NN::Runtime::Render::NNAddressMode::Clamp:  return ::Diligent::TEXTURE_ADDRESS_CLAMP;
                case NN::Runtime::Render::NNAddressMode::Mirror: return ::Diligent::TEXTURE_ADDRESS_MIRROR;
                case NN::Runtime::Render::NNAddressMode::Border: return ::Diligent::TEXTURE_ADDRESS_BORDER;
                default: return ::Diligent::TEXTURE_ADDRESS_WRAP;
            }
        };

        ::Diligent::SamplerDesc samplerDesc;
        samplerDesc.Name = "NNSampler";
        samplerDesc.MinFilter = ToDiligentFilter(desc.MinFilter);
        samplerDesc.MagFilter = ToDiligentFilter(desc.MagFilter);
        samplerDesc.MipFilter = ToDiligentFilter(desc.MipFilter);
        samplerDesc.AddressU = ToDiligentAddress(desc.AddressU);
        samplerDesc.AddressV = ToDiligentAddress(desc.AddressV);
        samplerDesc.AddressW = ToDiligentAddress(desc.AddressW);
        samplerDesc.MaxAnisotropy = desc.MaxAnisotropy;
        samplerDesc.MipLODBias = desc.MipLODBias;
        samplerDesc.MinLOD = desc.MinLOD;
        samplerDesc.MaxLOD = desc.MaxLOD;

        ::Diligent::ISampler* sampler = nullptr;
        m_Device->CreateSampler(samplerDesc, &sampler);
        if (!sampler) return {};

        auto* wrapper = new NNDiligentSampler(sampler, desc);
        wrapper->AddRef();
        sampler->Release();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNSampler>(wrapper);
    }

    // ===== CreateRenderTarget =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNRenderTarget> NNDiligentDevice::CreateRenderTarget(
        const NN::Runtime::Render::NNRenderTargetDesc& desc)
    {
        if (!m_Device) return {};

        auto* rt = new NNDiligentRenderTarget();
        if (!rt->Initialize(m_Device, desc))
        {
            delete rt;
            return {};
        }

        rt->AddRef();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNRenderTarget>(rt);
    }

    // ===== CreateShader =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNShader> NNDiligentDevice::CreateShader(
        const NN::Runtime::Render::NNShaderDesc& desc)
    {
        if (!m_Device) return {};

        ::Diligent::ShaderCreateInfo ci;
        ci.SourceLanguage = ::Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        ci.Desc.UseCombinedTextureSamplers = true;

        // Map NNShaderStage to Diligent SHADER_TYPE
        switch (desc.Stage)
        {
            case NN::Runtime::Render::NNShaderStage::Vertex:   ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_VERTEX;   break;
            case NN::Runtime::Render::NNShaderStage::Pixel:    ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_PIXEL;    break;
            case NN::Runtime::Render::NNShaderStage::Geometry: ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_GEOMETRY; break;
            case NN::Runtime::Render::NNShaderStage::Hull:     ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_HULL;     break;
            case NN::Runtime::Render::NNShaderStage::Domain:   ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_DOMAIN;   break;
            case NN::Runtime::Render::NNShaderStage::Compute:  ci.Desc.ShaderType = ::Diligent::SHADER_TYPE_COMPUTE;  break;
        }

        ci.EntryPoint = desc.EntryPoint ? desc.EntryPoint : "main";
        ci.Desc.Name = desc.DebugName ? desc.DebugName : "NNShader";

        if (desc.SourceCode)
        {
            ci.Source = desc.SourceCode;
        }
        else if (desc.ByteCode)
        {
            ci.ByteCode = desc.ByteCode;
            ci.ByteCodeSize = desc.ByteCodeSize;
        }

        ::Diligent::IShader* shader = nullptr;
        m_Device->CreateShader(ci, &shader, nullptr);
        if (!shader) return {};

        // NNDiligentShader takes ownership of the Diligent shader reference
        auto* wrapper = new NNDiligentShader(shader, desc.Stage, desc.DebugName);
        wrapper->AddRef();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNShader>(wrapper);
    }

    // ===== CreatePipelineState =====
    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNPipelineState> NNDiligentDevice::CreatePipelineState(
        const NN::Runtime::Render::NNPipelineStateDesc& desc)
    {
        if (!m_Device) return {};

        // Get Diligent shader objects from wrappers
        auto* vs = static_cast<NNDiligentShader*>(desc.VS);
        auto* ps = static_cast<NNDiligentShader*>(desc.PS);
        if (!vs || !ps) return {};

        ::Diligent::GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name = desc.DebugName ? desc.DebugName : "NNPSO";
        psoCI.PSODesc.PipelineType = ::Diligent::PIPELINE_TYPE_GRAPHICS;

        // Shaders
        psoCI.pVS = vs->GetDiligentShader();
        psoCI.pPS = ps->GetDiligentShader();

        // Input layout -- map NNVertexLayout to Diligent LayoutElements
        auto& layout = psoCI.GraphicsPipeline.InputLayout;
        std::vector<::Diligent::LayoutElement> elements;
        for (const auto& attr : desc.VertexLayout.Attributes)
        {
            ::Diligent::LayoutElement elem;
            elem.HLSLSemantic = attr.SemanticName;
            elem.InputIndex = attr.Location;
            elem.BufferSlot = 0;
            elem.RelativeOffset = attr.Offset;
            elem.Stride = desc.VertexLayout.Stride;
            elem.Frequency = (desc.VertexLayout.InputRate == NN::Runtime::Render::NNVertexInputRate::PerInstance)
                ? ::Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE : ::Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;

            // Map NNVertexFormat to Diligent ValueType + NumComponents
            switch (attr.Format)
            {
                case NN::Runtime::Render::NNVertexFormat::Float:       elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 1; break;
                case NN::Runtime::Render::NNVertexFormat::Float2:      elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 2; break;
                case NN::Runtime::Render::NNVertexFormat::Float3:      elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 3; break;
                case NN::Runtime::Render::NNVertexFormat::Float4:      elem.ValueType = ::Diligent::VT_FLOAT32; elem.NumComponents = 4; break;
                case NN::Runtime::Render::NNVertexFormat::Int:         elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 1; break;
                case NN::Runtime::Render::NNVertexFormat::Int2:        elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 2; break;
                case NN::Runtime::Render::NNVertexFormat::Int3:        elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 3; break;
                case NN::Runtime::Render::NNVertexFormat::Int4:        elem.ValueType = ::Diligent::VT_INT32;   elem.NumComponents = 4; break;
                case NN::Runtime::Render::NNVertexFormat::UByte4_Norm: elem.ValueType = ::Diligent::VT_UINT8;  elem.NumComponents = 4; elem.IsNormalized = true; break;
            }
            elements.push_back(elem);
        }
        layout.LayoutElements = elements.data();
        layout.NumElements = static_cast<::Diligent::Uint32>(elements.size());

        // Rasterizer state
        auto& raster = psoCI.GraphicsPipeline.RasterizerDesc;
        raster.FillMode = (desc.RasterizerState.FillMode == NN::Runtime::Render::NNFillMode::Wireframe)
            ? ::Diligent::FILL_MODE_WIREFRAME : ::Diligent::FILL_MODE_SOLID;
        raster.CullMode = (desc.RasterizerState.CullMode == NN::Runtime::Render::NNCullMode::None)
            ? ::Diligent::CULL_MODE_NONE
            : (desc.RasterizerState.CullMode == NN::Runtime::Render::NNCullMode::Front)
                ? ::Diligent::CULL_MODE_FRONT : ::Diligent::CULL_MODE_BACK;

        // Blend state
        auto& blend = psoCI.GraphicsPipeline.BlendDesc;
        ::Diligent::RenderTargetBlendDesc rtBlend;
        rtBlend.BlendEnable = desc.BlendState.Enable;
        if (desc.BlendState.Enable)
        {
            auto MapBlendFactor = [](NN::Runtime::Render::NNBlendFactor f) -> ::Diligent::BLEND_FACTOR
            {
                switch (f)
                {
                    case NN::Runtime::Render::NNBlendFactor::Zero:       return ::Diligent::BLEND_FACTOR_ZERO;
                    case NN::Runtime::Render::NNBlendFactor::One:        return ::Diligent::BLEND_FACTOR_ONE;
                    case NN::Runtime::Render::NNBlendFactor::SrcAlpha:   return ::Diligent::BLEND_FACTOR_SRC_ALPHA;
                    case NN::Runtime::Render::NNBlendFactor::InvSrcAlpha: return ::Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
                    default: return ::Diligent::BLEND_FACTOR_ONE;
                }
            };
            auto MapBlendOp = [](NN::Runtime::Render::NNBlendOp op) -> ::Diligent::BLEND_OPERATION
            {
                switch (op)
                {
                    case NN::Runtime::Render::NNBlendOp::Add:      return ::Diligent::BLEND_OPERATION_ADD;
                    case NN::Runtime::Render::NNBlendOp::Subtract: return ::Diligent::BLEND_OPERATION_SUBTRACT;
                    case NN::Runtime::Render::NNBlendOp::Min:      return ::Diligent::BLEND_OPERATION_MIN;
                    case NN::Runtime::Render::NNBlendOp::Max:      return ::Diligent::BLEND_OPERATION_MAX;
                    default: return ::Diligent::BLEND_OPERATION_ADD;
                }
            };
            rtBlend.SrcBlend = MapBlendFactor(desc.BlendState.SrcBlend);
            rtBlend.DestBlend = MapBlendFactor(desc.BlendState.DestBlend);
            rtBlend.BlendOp = MapBlendOp(desc.BlendState.BlendOp);
            rtBlend.SrcBlendAlpha = rtBlend.SrcBlend;
            rtBlend.DestBlendAlpha = rtBlend.DestBlend;
            rtBlend.BlendOpAlpha = rtBlend.BlendOp;
        }
        blend.RenderTargets[0] = rtBlend;

        // Depth stencil state
        auto& ds = psoCI.GraphicsPipeline.DepthStencilDesc;
        ds.DepthEnable = desc.DepthStencilState.DepthEnable;
        ds.DepthWriteEnable = desc.DepthStencilState.DepthWriteEnable;
        auto MapCmpFunc = [](NN::Runtime::Render::NNCompareFunc f) -> ::Diligent::COMPARISON_FUNCTION
        {
            switch (f)
            {
                case NN::Runtime::Render::NNCompareFunc::Never:        return ::Diligent::COMPARISON_FUNC_NEVER;
                case NN::Runtime::Render::NNCompareFunc::Less:         return ::Diligent::COMPARISON_FUNC_LESS;
                case NN::Runtime::Render::NNCompareFunc::Equal:        return ::Diligent::COMPARISON_FUNC_EQUAL;
                case NN::Runtime::Render::NNCompareFunc::LessEqual:    return ::Diligent::COMPARISON_FUNC_LESS_EQUAL;
                case NN::Runtime::Render::NNCompareFunc::Greater:      return ::Diligent::COMPARISON_FUNC_GREATER;
                case NN::Runtime::Render::NNCompareFunc::NotEqual:     return ::Diligent::COMPARISON_FUNC_NOT_EQUAL;
                case NN::Runtime::Render::NNCompareFunc::GreaterEqual: return ::Diligent::COMPARISON_FUNC_GREATER_EQUAL;
                case NN::Runtime::Render::NNCompareFunc::Always:       return ::Diligent::COMPARISON_FUNC_ALWAYS;
                default: return ::Diligent::COMPARISON_FUNC_LESS_EQUAL;
            }
        };
        ds.DepthFunc = MapCmpFunc(desc.DepthStencilState.DepthFunc);

        // Primitive topology
        psoCI.GraphicsPipeline.PrimitiveTopology = ::Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Render target formats - clear these since we use explicit RenderPass
        // (Diligent requires NumRenderTargets=0 when pRenderPass is set)
        psoCI.GraphicsPipeline.NumRenderTargets = 0;
        psoCI.GraphicsPipeline.RTVFormats[0] = ::Diligent::TEX_FORMAT_UNKNOWN;
        psoCI.GraphicsPipeline.DSVFormat = ::Diligent::TEX_FORMAT_UNKNOWN;

        // Sample count
        psoCI.GraphicsPipeline.SmplDesc.Count = desc.SampleCount;

        // Subpass index
        psoCI.GraphicsPipeline.SubpassIndex = 0;

        // Create a render pass matching the pipeline configuration.
        // If DSVFormat is Unknown, create a single-attachment (color only) render pass.
        const bool hasDepth = (ToDiligentFormat(desc.DSVFormat) != ::Diligent::TEX_FORMAT_UNKNOWN);

        // Color attachment
        ::Diligent::RenderPassAttachmentDesc colorAttach;
        colorAttach.Format = ToDiligentFormat(desc.RTVFormat);
        colorAttach.SampleCount = desc.SampleCount;
        colorAttach.LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
        colorAttach.StoreOp = ::Diligent::ATTACHMENT_STORE_OP_STORE;
        colorAttach.StencilLoadOp = ::Diligent::ATTACHMENT_LOAD_OP_DISCARD;
        colorAttach.StencilStoreOp = ::Diligent::ATTACHMENT_STORE_OP_DISCARD;
        colorAttach.InitialState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;
        colorAttach.FinalState = ::Diligent::RESOURCE_STATE_RENDER_TARGET;

        // Subpass references
        ::Diligent::AttachmentReference colorRef;
        colorRef.AttachmentIndex = 0;
        colorRef.State = ::Diligent::RESOURCE_STATE_RENDER_TARGET;

        ::Diligent::SubpassDesc subpass;
        subpass.RenderTargetAttachmentCount = 1;
        subpass.pRenderTargetAttachments = &colorRef;

        ::Diligent::RenderPassAttachmentDesc depthAttach;
        if (hasDepth)
        {
            depthAttach.Format = ToDiligentFormat(desc.DSVFormat);
            depthAttach.SampleCount = desc.SampleCount;
            depthAttach.LoadOp = ::Diligent::ATTACHMENT_LOAD_OP_CLEAR;
            depthAttach.StoreOp = ::Diligent::ATTACHMENT_STORE_OP_DISCARD;
            depthAttach.StencilLoadOp = ::Diligent::ATTACHMENT_LOAD_OP_DISCARD;
            depthAttach.StencilStoreOp = ::Diligent::ATTACHMENT_STORE_OP_DISCARD;
            depthAttach.InitialState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
            depthAttach.FinalState = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;

            ::Diligent::AttachmentReference depthRef;
            depthRef.AttachmentIndex = 1;
            depthRef.State = ::Diligent::RESOURCE_STATE_DEPTH_WRITE;
            subpass.pDepthStencilAttachment = &depthRef;
        }

        // Build render pass
        ::Diligent::RenderPassDesc rpDesc;
        rpDesc.Name = "NNDefaultRenderPass";
        ::Diligent::RenderPassAttachmentDesc attachments[2];
        attachments[0] = colorAttach;
        if (hasDepth) attachments[1] = depthAttach;
        rpDesc.pAttachments = attachments;
        rpDesc.AttachmentCount = hasDepth ? 2u : 1u;
        rpDesc.pSubpasses = &subpass;
        rpDesc.SubpassCount = 1;

        ::Diligent::IRenderPass* renderPass = nullptr;
        m_Device->CreateRenderPass(rpDesc, &renderPass);
        if (renderPass)
        {
            psoCI.GraphicsPipeline.pRenderPass = renderPass;
        }

        ::Diligent::IPipelineState* pso = nullptr;
        m_Device->CreatePipelineState(psoCI, &pso);
        if (!pso)
        {
            if (renderPass) renderPass->Release();
            return {};
        }

        // NNDiligentPipelineState takes ownership of both PSO and RenderPass references
        auto* wrapper = new NNDiligentPipelineState(pso, renderPass, desc);
        wrapper->AddRef();
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNPipelineState>(wrapper);
    }

    const NN::Runtime::Render::NNDeviceInfo& NNDiligentDevice::GetDeviceInfo() const { return m_DeviceInfo; }

    bool NNDiligentDevice::IsFeatureSupported(NN::Runtime::Render::NNFeature f) const
    {
        if (f == NN::Runtime::Render::NNFeature::Compute) return m_DeviceInfo.SupportsCompute;
        if (f == NN::Runtime::Render::NNFeature::RayTracing) return m_DeviceInfo.SupportsRayTracing;
        return false;
    }

    NN::Runtime::Render::INNCommandList* NNDiligentDevice::GetImmediateCommandList()
    {
        if (!m_ImmediateCmd) m_ImmediateCmd = NN::Runtime::Core::NNRef<NN::Runtime::Render::INNCommandList>(new NNDiligentCommandList(this, false));
        return m_ImmediateCmd.Get();
    }

    NN::Runtime::Core::NNRef<NN::Runtime::Render::INNCommandList> NNDiligentDevice::CreateDeferredCommandList()
    {
        return NN::Runtime::Core::NNRef<NN::Runtime::Render::INNCommandList>(new NNDiligentCommandList(this, true));
    }

    uint32_t NNDiligentDevice::AddRef() { return ++m_RefCount; }
    uint32_t NNDiligentDevice::Release() { uint32_t c = --m_RefCount; if (c == 0) delete this; return c; }
    uint32_t NNDiligentDevice::GetRefCount() const { return m_RefCount; }

} // namespace NNDiligent

