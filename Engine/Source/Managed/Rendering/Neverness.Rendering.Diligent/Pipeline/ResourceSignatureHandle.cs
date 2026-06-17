using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// PipelineResourceSignature 的句柄。
/// </summary>
public sealed class ResourceSignatureHandle : IDisposable
{
    internal IPipelineResourceSignature NativeObject { get; }

    internal ResourceSignatureHandle(IPipelineResourceSignature nativeSig)
    {
        NativeObject = nativeSig;
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
