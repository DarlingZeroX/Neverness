#pragma once

#include "../NNRenderConfig.h"
#include "NNDeviceInfo.h"
#include "../Resources/INNBuffer.h"
#include "../Resources/INNTexture.h"
#include "../Resources/INNSampler.h"
#include "../Pipeline/INNPipelineState.h"
#include "../Pipeline/INNShader.h"
#include "../RenderTarget/INNRenderTarget.h"
#include "../Command/INNCommandList.h"

#include <NNRuntimeCore/NNObject.h>

namespace NN::Runtime::Render
{
    class INNRenderDevice : public NN::Runtime::Core::INNObject
    {
    public:
        virtual NN::Runtime::Core::NNRef<INNBuffer> CreateBuffer(const NNBufferDesc& desc, const void* initialData = nullptr) = 0;
        virtual NN::Runtime::Core::NNRef<INNTexture> CreateTexture(const NNTextureDesc& desc, const void* initialData = nullptr) = 0;
        virtual NN::Runtime::Core::NNRef<INNShader> CreateShader(const NNShaderDesc& desc) = 0;
        virtual NN::Runtime::Core::NNRef<INNPipelineState> CreatePipelineState(const NNPipelineStateDesc& desc) = 0;
        virtual NN::Runtime::Core::NNRef<INNSampler> CreateSampler(const NNSamplerDesc& desc) = 0;
        virtual NN::Runtime::Core::NNRef<INNRenderTarget> CreateRenderTarget(const NNRenderTargetDesc& desc) = 0;

        virtual const NNDeviceInfo& GetDeviceInfo() const = 0;
        virtual bool IsFeatureSupported(NNFeature feature) const = 0;

        virtual INNCommandList* GetImmediateCommandList() = 0;
        virtual NN::Runtime::Core::NNRef<INNCommandList> CreateDeferredCommandList() = 0;
    };

} // namespace NN::Runtime::Render
