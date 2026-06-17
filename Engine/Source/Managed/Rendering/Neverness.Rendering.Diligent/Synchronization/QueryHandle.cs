using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Query 的句柄（当前 internal）。
/// </summary>
internal sealed class QueryHandle : IDisposable
{
    public IQuery NativeObject { get; }

    public QueryDesc Desc => NativeObject.GetDesc();

    internal QueryHandle(IQuery nativeQuery)
    {
        NativeObject = nativeQuery;
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
