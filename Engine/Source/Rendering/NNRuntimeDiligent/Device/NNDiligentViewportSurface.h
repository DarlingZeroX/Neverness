#pragma once

/**
 * @file NNDiligentViewportSurface.h
 * @brief 视口 Surface——管理单个 SwapChain 的生命周期。
 *
 * 职责：
 * - 从原生窗口句柄创建 Diligent SwapChain
 * - Deferred Resize（帧末统一执行 ResizeBuffers）
 * - Present 提交渲染结果
 * - Surface Lost 检测和重建
 *
 * 设计：
 * - 复用主窗口的 IRenderDevice / IDeviceContext
 * - 每个 Surface 持有独立 ISwapChain
 * - Device/Context 全局共享，由主窗口管理
 */

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <cstdint>

namespace NNDiligent
{
    using namespace NN::Runtime::Render;

    class NNDiligentViewportSurface
    {
    public:
        NNDiligentViewportSurface();
        ~NNDiligentViewportSurface();

        /**
         * @brief 创建 SwapChain。
         * @param device      Diligent 设备（来自主窗口）
         * @param context     Diligent 上下文（来自主窗口）
         * @param nativeHandle 原生窗口句柄
         * @param handleType   句柄类型（0=HWND, 1=X11, 2=Wayland, 3=NSView）
         * @param width        初始宽度
         * @param height       初始高度
         * @return true = 成功
         */
        bool Create(
            ::Diligent::IRenderDevice* device,
            ::Diligent::IDeviceContext* context,
            void* nativeHandle,
            uint32_t handleType,
            uint32_t width,
            uint32_t height);

        /** @brief 销毁 SwapChain。 */
        void Destroy();

        /** @brief 标记 Deferred Resize。 */
        void MarkResize(uint32_t width, uint32_t height);

        /** @brief 执行 Deferred ResizeBuffers。 */
        void FlushResize();

        /** @brief Present SwapChain。 */
        void Present();

        /** @brief 表面是否丢失。 */
        bool IsSurfaceLost() const { return m_SurfaceLost; }

        /**
         * @brief 重建丢失的表面。
         * @param newHandle     新原生窗口句柄
         * @param newHandleType 新句柄类型
         * @return true = 成功
         */
        bool Recreate(void* newHandle, uint32_t newHandleType);

        /** @brief 获取 SwapChain（供渲染使用）。 */
        ::Diligent::ISwapChain* GetSwapChain() { return m_SwapChain; }

        /** @brief 获取 Diligent 设备（来自主窗口，不持有所有权）。 */
        ::Diligent::IRenderDevice* GetDevice() { return m_Device; }

        /** @brief 获取 Diligent 上下文（来自主窗口，不持有所有权）。 */
        ::Diligent::IDeviceContext* GetContext() { return m_Context; }

        /** @brief Flush immediate context（提交所有 pending 命令）。 */
        void Flush() { if (m_Context) m_Context->Flush(); }

        /** @brief 是否已创建。 */
        bool IsCreated() const { return m_SwapChain != nullptr; }

    private:
        ::Diligent::IRenderDevice*  m_Device   = nullptr;  // 不持有引用（来自主窗口）
        ::Diligent::IDeviceContext* m_Context   = nullptr;  // 不持有引用（来自主窗口）
        ::Diligent::ISwapChain*     m_SwapChain = nullptr;

        uint32_t m_Width  = 0;
        uint32_t m_Height = 0;
        uint32_t m_PendingWidth  = 0;
        uint32_t m_PendingHeight = 0;
        bool     m_ResizePending = false;
        bool     m_SurfaceLost   = false;

        /** @brief Diligent 后端类型（记录用于重建）。 */
        enum class BackendType { Unknown, Vulkan, D3D12, D3D11, OpenGL };
        BackendType m_Backend = BackendType::Unknown;

        /** @brief 从原生句柄构建 Diligent::NativeWindow。 */
        static ::Diligent::NativeWindow BuildNativeWindow(void* handle, uint32_t handleType);

        /** @brief 创建 SwapChain（内部使用）。 */
        bool CreateSwapChain(::Diligent::NativeWindow nw, uint32_t width, uint32_t height);
    };

} // namespace NNDiligent
