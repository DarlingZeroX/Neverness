using Avalonia.Media;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 编辑器主题常量——统一颜色和样式。
/// </summary>
internal static class EditorTheme
{
    // ── 颜色 ──
    public static readonly Color TextPrimary = Color.Parse("#FFCCCCCC");
    public static readonly Color TextSecondary = Color.Parse("#FF808080");
    public static readonly Color Separator = Color.Parse("#FF3F3F46");
    public static readonly Color Warning = Color.Parse("#FFFF6600");

    // ── 画刷 ──
    public static readonly IBrush TextPrimaryBrush = new SolidColorBrush(TextPrimary);
    public static readonly IBrush TextSecondaryBrush = new SolidColorBrush(TextSecondary);
    public static readonly IBrush SeparatorBrush = new SolidColorBrush(Separator);
    public static readonly IBrush WarningBrush = new SolidColorBrush(Warning);

    // ── 尺寸 ──
    public const int LabelMinWidth = 120;
    public const int NumericWidth = 80;
    public const int ComboBoxMinWidth = 150;
    public const int TextBoxMinWidth = 200;
    public const int RowMinHeight = 28;
    public const double LabelFontSize = 12;
    public const double GroupFontSize = 13;
    public const double TitleFontSize = 18;
}
