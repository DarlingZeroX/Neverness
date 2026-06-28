// ============================================================================
// Window.cs - 窗口静态 API
// ============================================================================
// 窗口静态 API，通过 ViewportIdManager 获取当前 Viewport 的 SdlWindow。
// ============================================================================

using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Public;

namespace Neverness.Gameplay;

/// <summary>
/// 窗口静态 API。
/// </summary>
/// <remarks>
/// 通过 ViewportIdManager 获取当前 Viewport 的 SdlWindow。
///
/// 使用示例：
/// <code>
/// public class PlayerController : EntityBehaviour
/// {
///     public override void OnUpdate(float deltaTime)
///     {
///         var (w, h) = Window.Size;
///         var pos = Window.Position;
///     }
/// }
/// </code>
/// </remarks>
public static class Window
{
    // ========================================================================
    // 当前 Viewport 窗口
    // ========================================================================

    /// <summary>当前 Viewport 的 SdlWindow（可能为 null）。</summary>
    public static SdlWindow? Current
    {
        get
        {
            var viewport = ViewportIdManager.GetGameViewportId();
            return viewport.IsValid ? viewport.Window : null;
        }
    }

    /// <summary>当前窗口是否可用。</summary>
    public static bool IsAvailable => Current != null;

    // ========================================================================
    // 窗口属性
    // ========================================================================

    /// <summary>窗口尺寸。</summary>
    public static (int Width, int Height) Size
    {
        get => Current?.Size ?? (0, 0);
        set { if (Current != null) Current.Size = value; }
    }

    /// <summary>窗口位置。</summary>
    public static (int X, int Y) Position
    {
        get => Current?.Position ?? (0, 0);
        set { if (Current != null) Current.Position = value; }
    }

    /// <summary>平台原生窗口句柄（HWND / NSWindow / X11）。</summary>
    public static IntPtr NativeHandle => Current?.NativeHandle ?? IntPtr.Zero;

    // ========================================================================
    // 窗口操作
    // ========================================================================

    /// <summary>设置窗口标题。</summary>
    public static void SetTitle(string title) => Current?.SetTitle(title);

    /// <summary>设置窗口尺寸。</summary>
    public static void SetSize(int width, int height) => Current?.SetSize(width, height);

    /// <summary>设置窗口位置。</summary>
    public static void SetPosition(int x, int y) => Current?.SetPosition(x, y);

    /// <summary>显示窗口。</summary>
    public static void Show() => Current?.Show();

    /// <summary>隐藏窗口。</summary>
    public static void Hide() => Current?.Hide();

    /// <summary>最大化窗口。</summary>
    public static void Maximize() => Current?.Maximize();

    /// <summary>最小化窗口。</summary>
    public static void Minimize() => Current?.Minimize();

    /// <summary>恢复窗口。</summary>
    public static void Restore() => Current?.Restore();

    // ========================================================================
    // 事件快捷访问
    // ========================================================================

    /// <summary>窗口事件分发器（可能为 null）。</summary>
    public static SdlWindowEvents? Events => Current?.Events;
}
