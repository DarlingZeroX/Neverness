#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Resources/INNBuffer.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentBuffer : public INNBuffer
    {
    public:
        NNDiligentBuffer(::Diligent::IBuffer* buffer, const NNBufferDesc& desc);
        ~NNDiligentBuffer() override;

        // INNBuffer
        const NNBufferDesc& GetDesc() const override;
        void UpdateData(const void* data, uint32_t size, uint32_t offset = 0) override;
        uint32_t GetSize() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

        // Internal
        ::Diligent::IBuffer* GetDiligentBuffer() const { return m_Buffer; }

        // Set device context for dynamic buffer updates
        void SetDeviceContext(::Diligent::IDeviceContext* ctx) { m_Context = ctx; }

    private:
        ::Diligent::IBuffer*    m_Buffer = nullptr;
        ::Diligent::IDeviceContext* m_Context = nullptr; // for dynamic updates
        NNBufferDesc            m_Desc;
        uint32_t                m_Size = 0;
        std::atomic<uint32_t>   m_RefCount{0};
    };

} // namespace NNDiligent
