// NNDiligentCommandList.cpp

#include "../../Command/NNDiligentCommandList.h"
#include "../../Device/NNDiligentDevice.h"
#include "../../Pipeline/NNDiligentPipelineState.h"
#include "../../Resources/NNDiligentBuffer.h"
#include "../../NNDiligentConfig.h"
#include <iostream>
#include <vector>

namespace nnr = NN::Runtime::Render;
namespace nnc = NN::Runtime::Core;

NNDiligent::NNDiligentCommandList::NNDiligentCommandList(NNDiligentDevice* device, bool deferred)
    : m_Device(device), m_Deferred(deferred) {}

NNDiligent::NNDiligentCommandList::~NNDiligentCommandList() {}

void NNDiligent::NNDiligentCommandList::Begin() {}
void NNDiligent::NNDiligentCommandList::End() {}
void NNDiligent::NNDiligentCommandList::Reset() {}
void NNDiligent::NNDiligentCommandList::SetPipelineState(nnr::INNPipelineState* pso)
{
    if (!pso) return;
    auto* diliPso = static_cast<NNDiligentPipelineState*>(pso);
    m_Device->GetDiligentContext()->SetPipelineState(diliPso->GetDiligentPSO());
}

void NNDiligent::NNDiligentCommandList::SetVertexBuffer(nnr::INNBuffer* buffer, uint32_t slot)
{
    if (!buffer) return;
    auto* diliBuf = static_cast<NNDiligentBuffer*>(buffer);
    ::Diligent::IBuffer* buf = diliBuf->GetDiligentBuffer();
    ::Diligent::Uint64 offset = 0;
    m_Device->GetDiligentContext()->SetVertexBuffers(slot, 1, &buf, &offset,
        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        ::Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
}

void NNDiligent::NNDiligentCommandList::SetIndexBuffer(nnr::INNBuffer* buffer)
{
    if (!buffer) return;
    auto* diliBuf = static_cast<NNDiligentBuffer*>(buffer);
    m_Device->GetDiligentContext()->SetIndexBuffer(diliBuf->GetDiligentBuffer(), 0,
        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void NNDiligent::NNDiligentCommandList::Draw(const nnr::NNDrawAttribs& attribs)
{
    ::Diligent::DrawAttribs da;
    da.NumVertices           = attribs.VertexCount;
    da.StartVertexLocation   = attribs.StartVertexLocation;
    da.NumInstances          = attribs.InstanceCount;
    da.FirstInstanceLocation = attribs.StartInstanceLocation;
    m_Device->GetDiligentContext()->Draw(da);
}

void NNDiligent::NNDiligentCommandList::DrawIndexed(const nnr::NNDrawIndexedAttribs& attribs)
{
    ::Diligent::DrawIndexedAttribs da;
    da.NumIndices           = attribs.IndexCount;
    da.FirstIndexLocation   = attribs.StartIndexLocation;
    da.BaseVertex           = attribs.BaseVertexLocation;
    da.NumInstances         = attribs.InstanceCount;
    da.FirstInstanceLocation = attribs.StartInstanceLocation;
    m_Device->GetDiligentContext()->DrawIndexed(da);
}

void NNDiligent::NNDiligentCommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    ::Diligent::DispatchComputeAttribs da;
    da.ThreadGroupCountX = x;
    da.ThreadGroupCountY = y;
    da.ThreadGroupCountZ = z;
    m_Device->GetDiligentContext()->DispatchCompute(da);
}

void NNDiligent::NNDiligentCommandList::SetViewports(const nnr::NNViewport* viewports, uint32_t count)
{
    if (!viewports || count == 0) return;
    std::vector<::Diligent::Viewport> vps(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        vps[i].TopLeftX = viewports[i].TopLeftX;
        vps[i].TopLeftY = viewports[i].TopLeftY;
        vps[i].Width    = viewports[i].Width;
        vps[i].Height   = viewports[i].Height;
        vps[i].MinDepth = viewports[i].MinDepth;
        vps[i].MaxDepth = viewports[i].MaxDepth;
    }
    m_Device->GetDiligentContext()->SetViewports(count, vps.data(), 0, 0);
}

void NNDiligent::NNDiligentCommandList::SetScissorRects(const nnr::NNRect* rects, uint32_t count)
{
    if (!rects || count == 0) return;
    std::vector<::Diligent::Rect> r(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        r[i].left   = rects[i].Left;
        r[i].top    = rects[i].Top;
        r[i].right  = rects[i].Right;
        r[i].bottom = rects[i].Bottom;
    }
    m_Device->GetDiligentContext()->SetScissorRects(count, r.data(), 0, 0);
}

void NNDiligent::NNDiligentCommandList::ClearRenderTarget(float r, float g, float b, float a)
{
    auto* ctx = m_Device->GetDiligentContext();
    auto* sc = m_Device->GetDiligentSwapChain();
    if (!ctx || !sc) return;

    ::Diligent::ITextureView* pRTV = sc->GetCurrentBackBufferRTV();
    const float cc[] = {r, g, b, a};
    ctx->ClearRenderTarget(pRTV, cc, ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    ::Diligent::ITextureView* pDSV = sc->GetDepthBufferDSV();
    ctx->ClearDepthStencil(pDSV, ::Diligent::CLEAR_DEPTH_FLAG, 1.f, 0,
        ::Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void NNDiligent::NNDiligentCommandList::Present()
{
    auto* sc = m_Device->GetDiligentSwapChain();
    if (sc) sc->Present();
}

uint32_t NNDiligent::NNDiligentCommandList::AddRef() { return ++m_RefCount; }
uint32_t NNDiligent::NNDiligentCommandList::Release() { uint32_t c = --m_RefCount; if (c == 0) delete this; return c; }
uint32_t NNDiligent::NNDiligentCommandList::GetRefCount() const { return m_RefCount; }

