using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Shader 的句柄。
/// </summary>
public sealed class ShaderHandle : IDisposable
{
    internal IShader NativeObject { get; }

    internal ShaderHandle(IShader nativeShader)
    {
        NativeObject = nativeShader;
    }

    /// <summary>获取 Shader 字节码。</summary>
    public ReadOnlySpan<byte> GetBytecode()
    {
        return NativeObject.GetBytecode();
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
