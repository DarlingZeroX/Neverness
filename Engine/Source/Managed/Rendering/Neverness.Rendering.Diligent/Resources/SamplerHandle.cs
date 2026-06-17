using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Sampler 的句柄。
/// </summary>
public sealed class SamplerHandle : IDisposable
{
    internal ISampler NativeObject { get; }

    public SamplerDesc Desc => NativeObject.GetDesc();

    internal SamplerHandle(ISampler nativeSampler)
    {
        NativeObject = nativeSampler;
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
