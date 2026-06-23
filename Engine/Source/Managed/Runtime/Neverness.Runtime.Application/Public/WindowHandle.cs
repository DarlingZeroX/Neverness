// Neverness.Runtime.Application — 基于 SDL_WindowID 的窗口句柄。
// 替代 NNWindowHandle (ulong 自增)，直接使用 SDL3 原生窗口 ID。

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 窗口句柄，底层为 SDL_WindowID (uint32_t)。
/// 通过 SDL_GetWindowID() 获取，无需维护双重映射。
/// </summary>
public readonly struct WindowHandle : IEquatable<WindowHandle>
{
    /// <summary>无效句柄。</summary>
    public static readonly WindowHandle Invalid = new(0);

    /// <summary>SDL_WindowID 值。</summary>
    public uint Value { get; }

    public WindowHandle(uint value) => Value = value;

    /// <summary>是否有效（非零）。</summary>
    public bool IsValid => Value != 0;

    public bool Equals(WindowHandle other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is WindowHandle h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public override string ToString() => $"WindowHandle({Value})";

    public static bool operator ==(WindowHandle left, WindowHandle right) => left.Equals(right);
    public static bool operator !=(WindowHandle left, WindowHandle right) => !left.Equals(right);
}
