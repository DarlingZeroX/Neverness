using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser;

/// <summary>
/// 场景浏览器颜色常量。
/// </summary>
internal static class SceneBrowserColors
{
    // ── 背景色 ──
    internal static readonly IBrush BgMain = new SolidColorBrush(Color.Parse("#FF252526"));
    internal static readonly IBrush BgToolbar = new SolidColorBrush(Color.Parse("#FF2D2D30"));
    internal static readonly IBrush BgStatusBar = new SolidColorBrush(Color.Parse("#FF1E1E1E"));
    internal static readonly IBrush BgInput = new SolidColorBrush(Color.Parse("#FF1E1E1E"));

    // ── 文字色 ──
    internal static readonly IBrush TextPrimary = new SolidColorBrush(Color.Parse("#FFCCCCCC"));
    internal static readonly IBrush TextSecondary = new SolidColorBrush(Color.Parse("#FF888888"));
    internal static readonly IBrush TextBright = new SolidColorBrush(Color.Parse("#FFFFFFFF"));
    internal static readonly IBrush IconColor = new SolidColorBrush(Color.Parse("#FF7CB7FF"));

    // ── 分割线 ──
    internal static readonly IBrush Separator = new SolidColorBrush(Color.Parse("#FF3F3F46"));

    // ── 按钮 ──
    internal static readonly IBrush ButtonBg = new SolidColorBrush(Color.Parse("#FF3C3C3C"));
    internal static readonly IBrush ButtonHover = new SolidColorBrush(Color.Parse("#FF4A4A4A"));

    // ── 选中 ──
    internal static readonly IBrush SelectedBg = new SolidColorBrush(Color.FromArgb(0x4D, 0x21, 0x96, 0xF3));
}
