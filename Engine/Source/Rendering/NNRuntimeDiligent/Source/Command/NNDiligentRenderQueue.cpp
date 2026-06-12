// NNDiligentRenderQueue.cpp -- Diligent render queue implementation

#include "../../Command/NNDiligentRenderQueue.h"
#include "../../Device/NNDiligentDevice.h"

namespace NNDiligent
{
    NNDiligentRenderQueue::NNDiligentRenderQueue(NNDiligentDevice* device)
        : m_Device(device)
    {
    }

    NNDiligentRenderQueue::~NNDiligentRenderQueue()
    {
        // Release any pending commands still in the queue
        for (auto* cmd : m_PendingCmds)
        {
            if (cmd) cmd->Release();
        }
        m_PendingCmds.clear();
    }

    void NNDiligentRenderQueue::Submit(INNCommandList* cmd)
    {
        if (cmd)
        {
            cmd->AddRef();
            m_PendingCmds.push_back(cmd);
        }
    }

    void NNDiligentRenderQueue::Flush()
    {
        // For immediate context, commands are already executed when submitted.
        // Just release references and clear the pending list.
        for (auto* cmd : m_PendingCmds)
        {
            if (cmd) cmd->Release();
        }
        m_PendingCmds.clear();
    }

    uint32_t NNDiligentRenderQueue::GetPendingCount() const
    {
        return static_cast<uint32_t>(m_PendingCmds.size());
    }

    uint32_t NNDiligentRenderQueue::AddRef() { return ++m_RefCount; }
    uint32_t NNDiligentRenderQueue::Release()
    {
        uint32_t c = --m_RefCount;
        if (c == 0) delete this;
        return c;
    }
    uint32_t NNDiligentRenderQueue::GetRefCount() const { return m_RefCount; }

} // namespace NNDiligent
