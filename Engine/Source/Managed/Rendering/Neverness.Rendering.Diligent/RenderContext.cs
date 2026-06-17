using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// 封装 IDeviceContext 的命令录制 API。
/// 只负责 Draw / Dispatch / Copy / RenderPass 等命令录制。
/// Map/Unmap 在 BufferHandle 上，不在这里。
/// </summary>
public sealed class RenderContext
{
    internal IDeviceContext NativeObject { get; }

    internal RenderContext(IDeviceContext nativeContext)
    {
        NativeObject = nativeContext;
    }

    // ═══════════════════════════════════════════
    //  管线绑定
    // ═══════════════════════════════════════════

    public void SetPipelineState(PipelineStateHandle pso)
    {
        NativeObject.SetPipelineState(pso.NativeObject);
    }

    public void CommitShaderResources(ShaderResourceBindingHandle srb, ResourceStateTransitionMode mode)
    {
        NativeObject.CommitShaderResources(srb.NativeObject, mode);
    }

    // ═══════════════════════════════════════════
    //  顶点/索引缓冲
    // ═══════════════════════════════════════════

    public void SetVertexBuffers(uint startSlot, BufferHandle[] buffers, ulong[] offsets, ResourceStateTransitionMode mode)
    {
        var nativeBuffers = new IBuffer[buffers.Length];
        for (int i = 0; i < buffers.Length; i++)
            nativeBuffers[i] = buffers[i].NativeObject;
        NativeObject.SetVertexBuffers(startSlot, nativeBuffers, offsets, mode);
    }

    public void SetIndexBuffer(BufferHandle indexBuffer, ulong byteOffset, ResourceStateTransitionMode mode)
    {
        NativeObject.SetIndexBuffer(indexBuffer.NativeObject, byteOffset, mode);
    }

    // ═══════════════════════════════════════════
    //  渲染目标
    // ═══════════════════════════════════════════

    public void SetRenderTargets(TextureView[] rtvs, TextureView dsv, ResourceStateTransitionMode mode)
    {
        var nativeRTVs = new ITextureView[rtvs.Length];
        for (int i = 0; i < rtvs.Length; i++)
            nativeRTVs[i] = rtvs[i].NativeObject;
        NativeObject.SetRenderTargets(nativeRTVs, dsv?.NativeObject, mode);
    }

    public void SetViewports(Viewport[] viewports, uint rtWidth, uint rtHeight)
    {
        NativeObject.SetViewports(viewports, rtWidth, rtHeight);
    }

    public void SetScissorRects(Rect[] rects, uint rtWidth, uint rtHeight)
    {
        NativeObject.SetScissorRects(rects, rtWidth, rtHeight);
    }

    // ═══════════════════════════════════════════
    //  RenderPass
    // ═══════════════════════════════════════════

    public void BeginRenderPass(in BeginRenderPassAttribs attribs)
    {
        NativeObject.BeginRenderPass(attribs);
    }

    public void NextSubpass()
    {
        NativeObject.NextSubpass();
    }

    public void EndRenderPass()
    {
        NativeObject.EndRenderPass();
    }

    // ═══════════════════════════════════════════
    //  Draw
    // ═══════════════════════════════════════════

    public void Draw(in DrawAttribs attribs)
    {
        NativeObject.Draw(attribs);
    }

    public void DrawIndexed(in DrawIndexedAttribs attribs)
    {
        NativeObject.DrawIndexed(attribs);
    }

    public void DrawIndirect(in DrawIndirectAttribs attribs)
    {
        NativeObject.DrawIndirect(attribs);
    }

    public void DrawIndexedIndirect(in DrawIndexedIndirectAttribs attribs)
    {
        NativeObject.DrawIndexedIndirect(attribs);
    }

    // ═══════════════════════════════════════════
    //  Compute
    // ═══════════════════════════════════════════

    public void DispatchCompute(in DispatchComputeAttribs attribs)
    {
        NativeObject.DispatchCompute(attribs);
    }

    // ═══════════════════════════════════════════
    //  Copy / Transfer
    // ═══════════════════════════════════════════

    public void CopyTexture(in CopyTextureAttribs attribs)
    {
        NativeObject.CopyTexture(attribs);
    }

    public void UpdateBuffer(BufferHandle buffer, ulong offset, ulong size, IntPtr data, ResourceStateTransitionMode mode)
    {
        NativeObject.UpdateBuffer(buffer.NativeObject, offset, size, data, mode);
    }

    // ═══════════════════════════════════════════
    //  状态管理
    // ═══════════════════════════════════════════

    public void InvalidateState()
    {
        NativeObject.InvalidateState();
    }

    public void Begin(uint immediateContextId)
    {
        NativeObject.Begin(immediateContextId);
    }

    // ═══════════════════════════════════════════
    //  internal: Fence / Barrier / Sync
    //  为 RenderGraph 执行层预留，暂不对外暴露
    // ═══════════════════════════════════════════

    internal void TransitionResourceStates(StateTransitionDesc[] barriers)
    {
        NativeObject.TransitionResourceStates(barriers);
    }

    internal void EnqueueSignal(FenceHandle fence, ulong value)
    {
        NativeObject.EnqueueSignal(fence.NativeObject, value);
    }

    internal void DeviceWaitForFence(FenceHandle fence, ulong value)
    {
        NativeObject.DeviceWaitForFence(fence.NativeObject, value);
    }

    internal void WaitForIdle()
    {
        NativeObject.WaitForIdle();
    }
}
