/**
 * @file NNDiligentViewportSurface.cpp
 * @brief 视口 Surface 实现——管理单个 SwapChain 的生命周期。
 */

#include "../../Device/NNDiligentViewportSurface.h"
#include <iostream>
#include <SDL3/SDL.h>

namespace NNDiligent
{
    NNDiligentViewportSurface::NNDiligentViewportSurface() = default;

    NNDiligentViewportSurface::~NNDiligentViewportSurface()
    {
        Destroy();
    }

    bool NNDiligentViewportSurface::Create(
        ::Diligent::IRenderDevice* device,
        ::Diligent::IDeviceContext* context,
        void* nativeHandle,
        uint32_t handleType,
        uint32_t width,
        uint32_t height)
    {
        if (m_SwapChain)
        {
            std::cerr << "[NNDiligentViewportSurface] 已创建，先销毁再重建" << std::endl;
            return false;
        }

        if (!device || !context || !nativeHandle)
        {
            std::cerr << "[NNDiligentViewportSurface] 参数无效" << std::endl;
            return false;
        }

        m_Device = device;
        m_Context = context;
        m_Width = width;
        m_Height = height;

        // 构建 Diligent::NativeWindow
        auto nw = BuildNativeWindow(nativeHandle, handleType);

        // 创建 SwapChain
        if (!CreateSwapChain(nw, width, height))
        {
            std::cerr << "[NNDiligentViewportSurface] SwapChain 创建失败" << std::endl;
            return false;
        }

        m_SurfaceLost = false;
        std::cout << "[NNDiligentViewportSurface] 创建成功: " << width << "x" << height << std::endl;
        return true;
    }

    void NNDiligentViewportSurface::Destroy()
    {
        if (m_SwapChain)
        {
            m_SwapChain->Release();
            m_SwapChain = nullptr;
        }
        m_Device = nullptr;
        m_Context = nullptr;
        m_Width = 0;
        m_Height = 0;
        m_PendingWidth = 0;
        m_PendingHeight = 0;
        m_ResizePending = false;
        m_SurfaceLost = false;
        m_Backend = BackendType::Unknown;
    }

    void NNDiligentViewportSurface::MarkResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        m_PendingWidth = width;
        m_PendingHeight = height;
        m_ResizePending = true;
    }

    void NNDiligentViewportSurface::FlushResize()
    {
        if (!m_ResizePending || !m_SwapChain)
            return;

        uint32_t w = m_PendingWidth;
        uint32_t h = m_PendingHeight;
        m_ResizePending = false;

        if (w == m_Width && h == m_Height)
            return;

        m_Width = w;
        m_Height = h;

        // Diligent SwapChain::Resize
        m_SwapChain->Resize(w, h);
        std::cout << "[NNDiligentViewportSurface] ResizeBuffers: " << w << "x" << h << std::endl;
    }

    void NNDiligentViewportSurface::Present()
    {
        if (m_SwapChain)
        {
            m_SwapChain->Present();
        }
    }

    bool NNDiligentViewportSurface::Recreate(void* newHandle, uint32_t newHandleType)
    {
        if (!m_Device || !m_Context)
        {
            std::cerr << "[NNDiligentViewportSurface] Recreate 失败：设备无效" << std::endl;
            return false;
        }

        // 销毁旧 SwapChain
        if (m_SwapChain)
        {
            m_SwapChain->Release();
            m_SwapChain = nullptr;
        }

        // 用新句柄重建
        auto nw = BuildNativeWindow(newHandle, newHandleType);
        if (!CreateSwapChain(nw, m_Width, m_Height))
        {
            std::cerr << "[NNDiligentViewportSurface] Recreate SwapChain 失败" << std::endl;
            m_SurfaceLost = true;
            return false;
        }

        m_SurfaceLost = false;
        std::cout << "[NNDiligentViewportSurface] SwapChain 重建成功" << std::endl;
        return true;
    }

    // ── 内部实现 ──

    ::Diligent::NativeWindow NNDiligentViewportSurface::BuildNativeWindow(void* handle, uint32_t handleType)
    {
        ::Diligent::NativeWindow nw{};

        switch (handleType)
        {
        case 0: // Win32HWND
#if PLATFORM_WIN32
            nw.hWnd = handle;
#endif
            break;

        case 1: // X11Window
#if PLATFORM_LINUX
            nw.WindowId = reinterpret_cast<uintptr_t>(handle);
#endif
            break;

        // TODO: Wayland (2) 和 NSView (3) 的支持
        default:
            std::cerr << "[NNDiligentViewportSurface] 不支持的句柄类型: " << handleType << std::endl;
            break;
        }

        return nw;
    }

    bool NNDiligentViewportSurface::CreateSwapChain(::Diligent::NativeWindow nw, uint32_t width, uint32_t height)
    {
        ::Diligent::SwapChainDesc scDesc;
        scDesc.Width = width;
        scDesc.Height = height;

        // 检测当前后端类型（从主设备推断）
        const auto& devInfo = m_Device->GetDeviceInfo();
        const char* backendName = devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_D3D12 ? "D3D12" :
                                  devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_D3D11 ? "D3D11" :
                                  devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_VULKAN ? "Vulkan" :
                                  devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_GL ? "OpenGL" : "Unknown";

        // 根据设备类型创建对应后端的 SwapChain
        bool ok = false;

#if D3D12_SUPPORTED
        if (devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_D3D12)
        {
            auto* F = ::Diligent::LoadAndGetEngineFactoryD3D12();
            if (F)
            {
                F->CreateSwapChainD3D12(m_Device, m_Context, scDesc, ::Diligent::FullScreenModeDesc{}, nw, &m_SwapChain);
                ok = (m_SwapChain != nullptr);
                m_Backend = BackendType::D3D12;
            }
        }
#endif
#if D3D11_SUPPORTED
        if (!ok && devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_D3D11)
        {
            auto* F = ::Diligent::LoadAndGetEngineFactoryD3D11();
            if (F)
            {
                F->CreateSwapChainD3D11(m_Device, m_Context, scDesc, ::Diligent::FullScreenModeDesc{}, nw, &m_SwapChain);
                ok = (m_SwapChain != nullptr);
                m_Backend = BackendType::D3D11;
            }
        }
#endif
#if VULKAN_SUPPORTED
        if (!ok && devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_VULKAN)
        {
            auto* F = ::Diligent::LoadAndGetEngineFactoryVk();
            if (F)
            {
                F->CreateSwapChainVk(m_Device, m_Context, scDesc, nw, &m_SwapChain);
                ok = (m_SwapChain != nullptr);
                m_Backend = BackendType::Vulkan;
            }
        }
#endif

        // OpenGL 不支持独立 SwapChain（设备+上下文+SwapChain 一体创建）
        // 且 Windows 上 OpenGL 后端库未链接，跳过
#if GL_SUPPORTED && !PLATFORM_WIN32
        if (!ok && devInfo.Type == ::Diligent::RENDER_DEVICE_TYPE_GL)
        {
            std::cerr << "[NNDiligentViewportSurface] OpenGL 不支持多 SwapChain（设备+SwapChain 一体创建）" << std::endl;
            m_Backend = BackendType::OpenGL;
            return false;
        }
#endif

        if (!ok)
        {
            std::cerr << "[NNDiligentViewportSurface] SwapChain 创建失败，后端: " << backendName << std::endl;
            return false;
        }

        std::cout << "[NNDiligentViewportSurface] SwapChain 已创建，后端: " << backendName
                  << ", 尺寸: " << width << "x" << height << std::endl;
        return true;
    }

} // namespace NNDiligent
