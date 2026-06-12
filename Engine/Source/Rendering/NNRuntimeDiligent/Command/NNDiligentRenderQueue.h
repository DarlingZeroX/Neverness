// NNDiligentRenderQueue.h -- Diligent render queue implementation

#pragma once

#include "../NNDiligentConfig.h"
#include <NNRuntimeRender/Command/INNRenderQueue.h>
#include <NNRuntimeCore/NNObject.h>
#include <atomic>
#include <vector>

namespace NNDiligent
{
    using namespace NN::Runtime::Core;
    using namespace NN::Runtime::Render;

    class NNDiligentDevice;

    class NNDiligentRenderQueue : public INNRenderQueue
    {
    public:
        NNDiligentRenderQueue(NNDiligentDevice* device);
        ~NNDiligentRenderQueue() override;

        // INNRenderQueue
        void Submit(INNCommandList* cmd) override;
        void Flush() override;
        uint32_t GetPendingCount() const override;

        // INNObject
        uint32_t AddRef() override;
        uint32_t Release() override;
        uint32_t GetRefCount() const override;

    private:
        NNDiligentDevice* m_Device;
        std::vector<INNCommandList*> m_PendingCmds;
        std::atomic<uint32_t> m_RefCount{0};
    };

} // namespace NNDiligent
