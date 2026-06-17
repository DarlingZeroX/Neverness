using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// PipelineState 的句柄。
/// </summary>
public sealed class PipelineStateHandle : IDisposable
{
    internal IPipelineState NativeObject { get; }

    internal PipelineStateHandle(IPipelineState nativePSO)
    {
        NativeObject = nativePSO;
    }

    /// <summary>创建 ShaderResourceBinding。</summary>
    public ShaderResourceBindingHandle CreateShaderResourceBinding(bool initStaticResources = true)
    {
        var nativeSRB = NativeObject.CreateShaderResourceBinding(initStaticResources);
        return new ShaderResourceBindingHandle(nativeSRB);
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
