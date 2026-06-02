#pragma once

#include "../NNRenderConfig.h"
#include "../Resources/INNBuffer.h"
#include "../Pipeline/INNPipelineState.h"
#include <NNRuntimeCore/NNObject.h>
#include <cstdint>

namespace NN::Runtime::Render
{
    struct NNDrawAttribs
    {
        uint32_t VertexCount = 0;
        uint32_t StartVertexLocation = 0;
        uint32_t InstanceCount = 1;
        uint32_t StartInstanceLocation = 0;
    };

    struct NNDrawIndexedAttribs
    {
        uint32_t IndexCount = 0;
        uint32_t StartIndexLocation = 0;
        int32_t BaseVertexLocation = 0;
        uint32_t InstanceCount = 1;
        uint32_t StartInstanceLocation = 0;
    };

    struct NNViewport
    {
        float TopLeftX = 0.0f;
        float TopLeftY = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0f;
    };

    struct NNRect
    {
        int32_t Left = 0;
        int32_t Top = 0;
        int32_t Right = 0;
        int32_t Bottom = 0;
    };

    class INNCommandList : public NN::Runtime::Core::INNObject
    {
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Reset() = 0;

        virtual void SetPipelineState(INNPipelineState* pso) = 0;
        virtual void SetVertexBuffer(INNBuffer* buffer, uint32_t slot = 0) = 0;
        virtual void SetIndexBuffer(INNBuffer* buffer) = 0;

        virtual void Draw(const NNDrawAttribs& attribs) = 0;
        virtual void DrawIndexed(const NNDrawIndexedAttribs& attribs) = 0;
        virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

        virtual void SetViewports(const NNViewport* viewports, uint32_t count) = 0;
        virtual void SetScissorRects(const NNRect* rects, uint32_t count) = 0;
    };

} // namespace NN::Runtime::Render
