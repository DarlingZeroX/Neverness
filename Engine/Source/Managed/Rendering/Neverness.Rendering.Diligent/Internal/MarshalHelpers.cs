using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Neverness.Rendering.Diligent.Internal;

/// <summary>
/// 结构体 marshalling 辅助方法。
/// </summary>
internal static class MarshalHelpers
{
    /// <summary>
    /// 将托管 span 转换为非托管指针（stackalloc 场景）。
    /// </summary>
    public static unsafe IntPtr ToUnmanagedPtr<T>(ReadOnlySpan<T> span) where T : unmanaged
    {
        fixed (T* ptr = span)
            return (IntPtr)ptr;
    }
}
