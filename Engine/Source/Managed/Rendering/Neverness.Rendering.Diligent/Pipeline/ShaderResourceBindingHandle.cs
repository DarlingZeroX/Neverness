using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// ShaderResourceBinding 的句柄。
/// </summary>
public sealed class ShaderResourceBindingHandle : IDisposable
{
    internal IShaderResourceBinding NativeObject { get; }

    internal ShaderResourceBindingHandle(IShaderResourceBinding nativeSRB)
    {
        NativeObject = nativeSRB;
    }

    /// <summary>获取 ShaderResourceVariable。</summary>
    public IShaderResourceVariable GetVariable(ShaderType shaderType, string name)
    {
        return NativeObject.GetVariableByName(shaderType, name);
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
