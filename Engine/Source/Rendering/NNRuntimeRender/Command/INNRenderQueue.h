#pragma once

#include "../NNRenderConfig.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Render
{
    class INNCommandList;

    class INNRenderQueue : public NN::Runtime::Core::INNObject
    {
    public:
        virtual void Submit(INNCommandList* cmd) = 0;
        virtual void Flush() = 0;
        virtual uint32_t GetPendingCount() const = 0;
    };

} // namespace NN::Runtime::Render
