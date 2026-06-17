using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Buffer 的轻量句柄。
/// 实现 IDisposable，析构时调用原生 Release。
/// 提供 Map/Unmap，支持 Persistent Mapping 和 Ring Buffer 模式。
/// </summary>
public sealed class BufferHandle : IDisposable
{
    internal IBuffer NativeObject { get; }

    public BufferDesc Desc => NativeObject.GetDesc();

    internal BufferHandle(IBuffer nativeBuffer)
    {
        NativeObject = nativeBuffer;
    }

    /// <summary>映射缓冲区到 CPU 可访问内存。</summary>
    public IntPtr Map(MapType mapType, MapFlags mapFlags)
    {
        // TODO: 需要 DeviceContext 来执行 Map 操作
        // Diligent 的 Map 是在 DeviceContext 上调用的，不是 Buffer 上
        // 这里先抛出 NotImplementedException，等 RenderContext 实现后再对接
        throw new NotImplementedException("Map 操作需要通过 RenderContext 执行，后续 Phase 实现");
    }

    /// <summary>取消映射。</summary>
    public void Unmap()
    {
        throw new NotImplementedException("Unmap 操作需要通过 RenderContext 执行，后续 Phase 实现");
    }

    /// <summary>便利方法：映射后写入数据。</summary>
    public unsafe void WriteData<T>(ReadOnlySpan<T> data) where T : unmanaged
    {
        throw new NotImplementedException("WriteData 需要通过 RenderContext 执行，后续 Phase 实现");
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
