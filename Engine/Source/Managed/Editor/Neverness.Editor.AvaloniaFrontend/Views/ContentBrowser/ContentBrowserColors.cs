using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器颜色常量和尺寸常量。
/// Unreal Content Browser 风格配色。
/// </summary>
internal static class ContentBrowserColors
{
    /* ======================== 主背景 ======================== */

    internal static readonly IBrush BgMain = new SolidColorBrush(Color.Parse("#FF2B2B2B"));
    internal static readonly IBrush BgPanel = new SolidColorBrush(Color.Parse("#FF353535"));
    internal static readonly IBrush BgToolbar = new SolidColorBrush(Color.Parse("#FF3B3B3B"));
    internal static readonly IBrush BgStatusBar = new SolidColorBrush(Color.Parse("#FF252525"));
    internal static readonly IBrush BgInput = new SolidColorBrush(Color.Parse("#FF1E1E1E"));

    /* ======================== 缩略图 ======================== */

    internal static readonly IBrush BgThumbFile = new SolidColorBrush(Color.Parse("#FF404040"));
    internal static readonly IBrush BgThumbDir = new SolidColorBrush(Color.Parse("#FF484848"));
    internal static readonly IBrush BgThumbHover = new SolidColorBrush(Color.Parse("#FF505050"));
    internal static readonly IBrush BgThumbSelected = new SolidColorBrush(Color.Parse("#FF2A5D9E"));

    /* ======================== 选区矩形 ======================== */

    internal static readonly IBrush SelectionFill = new SolidColorBrush(Color.FromArgb(0x30, 0x21, 0x96, 0xF3));
    internal static readonly IBrush SelectionStroke = new SolidColorBrush(Color.FromArgb(0x80, 0x21, 0x96, 0xF3));

    /* ======================== 选中边框 ======================== */

    internal static readonly IBrush BorderSelected = new SolidColorBrush(Color.Parse("#FF2196F3"));

    /* ======================== 文字 ======================== */

    internal static readonly IBrush TextPrimary = new SolidColorBrush(Color.Parse("#FFCCCCCC"));
    internal static readonly IBrush TextSecondary = new SolidColorBrush(Color.Parse("#FF888888"));
    internal static readonly IBrush TextBright = new SolidColorBrush(Color.Parse("#FFFFFFFF"));

    /* ======================== 分割线 ======================== */

    internal static readonly IBrush Separator = new SolidColorBrush(Color.Parse("#FF1A1A1A"));

    /* ======================== 类型标签颜色 ======================== */

    internal static readonly IBrush BadgeScene = new SolidColorBrush(Color.Parse("#FF4CAF50"));
    internal static readonly IBrush BadgePrefab = new SolidColorBrush(Color.Parse("#FF2196F3"));
    internal static readonly IBrush BadgeMaterial = new SolidColorBrush(Color.Parse("#FFFF9800"));
    internal static readonly IBrush BadgeTexture = new SolidColorBrush(Color.Parse("#FF9C27B0"));
    internal static readonly IBrush BadgeAudio = new SolidColorBrush(Color.Parse("#FFFF5722"));
    internal static readonly IBrush BadgeScript = new SolidColorBrush(Color.Parse("#FF00BCD4"));
    internal static readonly IBrush BadgeDefault = new SolidColorBrush(Color.Parse("#FF607D8B"));

    /* ======================== 缩略图尺寸 ======================== */

    internal const double ThumbIconSize = 80;   // 图标区域高度
    internal const double ThumbNameHeight = 16;  // 文件名行高
    internal const double ThumbTypeHeight = 12;  // 类型标签行高
    internal const double ThumbTextPad = 5;      // 文字区内边距
    internal const double ThumbCardWidth = 96;   // 卡片总宽
    internal const double ThumbCardHeight = ThumbIconSize + ThumbNameHeight + ThumbTypeHeight + ThumbTextPad * 2;
    internal const double ThumbSpacing = 8;
}
