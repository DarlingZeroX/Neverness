#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Device/INNRenderDevice.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    // 鍏抽敭: 杩欎袱涓?using namespace 璁╃被鍨嬪悕绉颁笌鎺ュ彛瀹屽叏鍖归厤
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentCommandList;

    class NNDiligentDevice : public INNRenderDevice
    {
    public:
        NNDiligentDevice();
        ~NNDiligentDevice() override;

        bool Initialize(const NNRenderDeviceCreateInfo& info);

        /// @brief 从平台原生窗口句柄初始化设备，绕过 SDL。
        /// @param nativeHandle 平台原生句柄（HWND / X11 Window / NSView 等）
        /// @param handleType 句柄类型：0=Win32HWND, 1=X11Window, 2=Wayland, 3=NSView
        bool InitializeFromNativeHandle(void* nativeHandle, uint32_t handleType,
                                        uint32_t width, uint32_t height,
                                        NNRenderBackendType backend = NNRenderBackendType::Backend_D3D12,
                                        bool enableValidation = true, bool vsync = true);

        void Shutdown();

        NNRef<INNBuffer> CreateBuffer(const NNBufferDesc& desc, const void* initialData = nullptr) override;
        NNRef<INNTexture> CreateTexture(const NNTextureDesc& desc, const void* initialData = nullptr) override;
        NNRef<INNShader> CreateShader(const NNShaderDesc& desc) override;
        NNRef<INNPipelineState> CreatePipelineState(const NNPipelineStateDesc& desc) override;
        NNRef<INNSampler> CreateSampler(const NNSamplerDesc& desc) override;
        NNRef<INNRenderTarget> CreateRenderTarget(const NNRenderTargetDesc& desc) override;

        const NNDeviceInfo& GetDeviceInfo() const override;
        bool IsFeatureSupported(NNFeature feature) const override;

        INNCommandList* GetImmediateCommandList() override;
        NNRef<INNCommandList> CreateDeferredCommandList() override;

        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        ::Diligent::IRenderDevice*  GetDiligentDevice()   { return m_Device; }
        ::Diligent::IDeviceContext* GetDiligentContext()   { return m_Context; }
        ::Diligent::ISwapChain*     GetDiligentSwapChain() { return m_SwapChain; }

    private:
        ::Diligent::IRenderDevice*  m_Device   = nullptr;
        ::Diligent::IDeviceContext* m_Context   = nullptr;
        ::Diligent::ISwapChain*     m_SwapChain = nullptr;

        NNDeviceInfo m_DeviceInfo{};
        std::atomic<uint32_t> m_RefCount{0};
        NNRef<INNCommandList> m_ImmediateCmd;
    };

} // namespace NNDiligent

