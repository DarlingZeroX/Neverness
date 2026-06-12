#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Command/INNCommandList.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentDevice;

    class NNDiligentCommandList : public INNCommandList
    {
    public:
        NNDiligentCommandList(NNDiligentDevice* device, bool deferred);
        ~NNDiligentCommandList() override;

        void Begin() override;
        void End() override;
        void Reset() override;

        void SetPipelineState(INNPipelineState* pso) override;
        void SetVertexBuffer(INNBuffer* buffer, uint32_t slot = 0) override;
        void SetIndexBuffer(INNBuffer* buffer) override;

        void Draw(const NNDrawAttribs& attribs) override;
        void DrawIndexed(const NNDrawIndexedAttribs& attribs) override;
        void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;

        void SetViewports(const NNViewport* viewports, uint32_t count) override;
        void SetScissorRects(const NNRect* rects, uint32_t count) override;

        void ClearRenderTarget(float r, float g, float b, float a);
        void Present();

        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

    private:
        NNDiligentDevice* m_Device;
        bool m_Deferred;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NNDiligent

