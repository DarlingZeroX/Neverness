using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Styling;

/// <summary>
/// 编辑器统一主题颜色——全编辑器共用的颜色常量。
/// </summary>
public static class EditorTheme
{
    // ── 背景色 ──
    public static readonly SolidColorBrush Background = new(Color.Parse("#FF1E1E1E"));
    public static readonly SolidColorBrush PanelBackground = new(Color.Parse("#FF252526"));
    public static readonly SolidColorBrush HeaderBackground = new(Color.Parse("#FF2D2D30"));
    public static readonly SolidColorBrush InputBackground = new(Color.Parse("#FF2D2D30"));
    public static readonly SolidColorBrush HoverBackground = new(Color.FromArgb(200, 70, 70, 78));

    // ── 边框色 ──
    public static readonly SolidColorBrush Border = new(Color.Parse("#FF3F3F46"));
    public static readonly SolidColorBrush BorderAccent = new(Color.Parse("#FF007ACC"));

    // ── 文字色 ──
    public static readonly SolidColorBrush TextPrimary = new(Color.Parse("#FFCCCCCC"));
    public static readonly SolidColorBrush TextSecondary = new(Color.Parse("#FF999999"));
    public static readonly SolidColorBrush TextDisabled = new(Color.Parse("#FF656565"));

    // ── 轴标签色（XYZ / RGBA）──
    public static readonly SolidColorBrush AxisX = new(Color.Parse("#FFF44336"));
    public static readonly SolidColorBrush AxisY = new(Color.Parse("#FF4CAF50"));
    public static readonly SolidColorBrush AxisZ = new(Color.Parse("#FF2196F3"));
    public static readonly SolidColorBrush AxisR = new(Color.Parse("#FFF44336"));
    public static readonly SolidColorBrush AxisG = new(Color.Parse("#FF4CAF50"));
    public static readonly SolidColorBrush AxisB = new(Color.Parse("#FF2196F3"));
    public static readonly SolidColorBrush AxisA = new(Color.Parse("#FFCCCCCC"));

    // ── 状态色 ──
    public static readonly SolidColorBrush Accent = new(Color.Parse("#FF007ACC"));
    public static readonly SolidColorBrush Success = new(Color.Parse("#FF4CAF50"));
    public static readonly SolidColorBrush Warning = new(Color.Parse("#FFFFC107"));
    public static readonly SolidColorBrush Error = new(Color.Parse("#FFF44336"));
}
