using System.Runtime.InteropServices;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 标签组件——存储实体名称和标志位。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct TagComponent : IComponent
{
    /// <summary>实体名称（固定 64 字节缓冲区）。</summary>
    private unsafe fixed char _name[32];

    /// <summary>标志位。</summary>
    public uint Flags;

    /// <summary>创建标签组件。</summary>
    public static TagComponent Create(string name)
    {
        var tag = new TagComponent();
        tag.SetName(name);
        return tag;
    }

    /// <summary>默认标签。</summary>
    public static TagComponent Default => new();

    /// <summary>获取或设置名称。</summary>
    public string Name
    {
        readonly get
        {
            unsafe
            {
                fixed (char* ptr = _name)
                {
                    return new string(ptr);
                }
            }
        }
        set => SetName(value);
    }

    private void SetName(string name)
    {
        unsafe
        {
            fixed (char* ptr = _name)
            {
                var span = new Span<char>(ptr, 32);
                name.AsSpan(0, Math.Min(name.Length, 31)).CopyTo(span);
                span[name.Length < 31 ? name.Length : 31] = '\0';
            }
        }
    }
}
