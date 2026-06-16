using Avalonia.Controls;
using Avalonia.Platform;

namespace Neverness.Editor.AvaloniaFrontend.Viewport;

/// <summary>
/// 自定义 NativeControlHost——重写 CreateNativeControlCore 获取原生子窗口句柄。
///
/// 工作原理：
/// 1. Avalonia 的 NativeControlHost 在附加到可视树时自动创建 DumbWindow 子窗口
/// 2. CreateNativeControlCore(parent) 的 parent 参数就是 DumbWindow 的 IPlatformHandle
/// 3. parent.Handle 就是 HWND（Windows）/ X11 Window / NSView
/// 4. 我们直接使用 parent，不额外 CreateWindowEx
///
/// 跨平台：
/// - Windows：parent.HandleDescriptor = "HWND"，parent.Handle = HWND
/// - Linux X11：parent.HandleDescriptor = "X11"，parent.Handle = X11 Window
/// - macOS：parent.HandleDescriptor = "NSView"，parent.Handle = NSView*
/// </summary>
public class ViewportHostControl : NativeControlHost
{
    /// <summary>原生句柄创建事件。</summary>
    public event Action<IPlatformHandle>? HandleCreated;

    /// <summary>原生句柄销毁事件。</summary>
    public event Action? HandleDestroyed;

    /// <summary>当前原生句柄。</summary>
    private IPlatformHandle? _currentHandle;

    /// <summary>获取当前原生句柄。</summary>
    public IPlatformHandle? CurrentHandle => _currentHandle;

    /// <summary>
    /// 重写 CreateNativeControlCore——直接使用 Avalonia 创建的 DumbWindow。
    ///
    /// Avalonia 内部流程：
    ///   UpdateHost() → CreateNewAttachment() → 创建 DumbWindow → 调用 CreateNativeControlCore(parent)
    ///   parent 参数就是 DumbWindow 的 IPlatformHandle
    /// </summary>
    protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
    {
        // 直接使用 parent（Avalonia 创建的 DumbWindow），不额外创建子窗口
        _currentHandle = parent;

        Console.WriteLine($"[ViewportHostControl] 子窗口已创建: Handle=0x{parent.Handle:X} ({parent.HandleDescriptor})");

        // 通知外部：原生句柄已就绪
        HandleCreated?.Invoke(parent);

        return parent;
    }

    /// <summary>
    /// 重写 DestroyNativeControlCore——清理句柄引用。
    ///
    /// Avalonia 在 NativeControlHost 从可视树移除时调用此方法。
    /// </summary>
    protected override void DestroyNativeControlCore(IPlatformHandle control)
    {
        Console.WriteLine($"[ViewportHostControl] 子窗口销毁: Handle=0x{control.Handle:X}");

        _currentHandle = null;

        // 通知外部：原生句柄已销毁
        HandleDestroyed?.Invoke();
    }
}
