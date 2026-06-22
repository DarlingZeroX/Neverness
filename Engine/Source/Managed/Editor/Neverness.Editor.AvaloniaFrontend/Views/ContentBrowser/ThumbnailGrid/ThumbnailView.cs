using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ThumbnailGrid;

/// <summary>
/// 缩略图纯 UI 控件——只负责创建视觉结构，不绑定交互事件。
/// 可被 ContentBrowser、AssetPicker、Inspector 等复用。
/// </summary>
internal sealed class ThumbnailView
{
    /// <summary>创建缩略图控件。</summary>
    /// <param name="name">显示名称</param>
    /// <param name="icon">图标 emoji</param>
    /// <param name="path">路径（作为 Tag 用于识别）</param>
    /// <param name="isDirectory">是否为目录</param>
    /// <param name="typeLabel">类型标签（如 TEX、MESH）</param>
    /// <param name="isSelected">是否选中</param>
    /// <param name="badgeColor">徽章颜色</param>
    /// <returns>外层 Border 容器，Tag 为 path</returns>
    internal static Control Create(string name, string icon, string path, bool isDirectory,
        string typeLabel, bool isSelected, IBrush? badgeColor = null)
    {
        // 外层容器（提供阴影空间 + Tag 用于事件识别）
        var shadowPad = 4; // 阴影扩展空间
        var outer = new Border
        {
            Width = ThumbCardWidth + shadowPad * 2,
            Height = ThumbCardHeight + shadowPad * 2,
            Margin = new Thickness(ThumbSpacing / 2 - shadowPad),
            Padding = new Thickness(shadowPad),
            Background = Brushes.Transparent,
            Tag = path,
        };

        // 卡片 Border（圆角 + 选中高亮 + 阴影）
        var card = new Border
        {
            Width = ThumbCardWidth,
            Height = ThumbCardHeight,
            CornerRadius = new CornerRadius(4),
            Background = isSelected ? BgThumbSelected : (isDirectory ? BgThumbDir : BgThumbFile),
            BorderBrush = isSelected ? BorderSelected : Brushes.Transparent,
            BorderThickness = new Thickness(isSelected ? 2 : 0),
            BoxShadow = new BoxShadows(new BoxShadow
            {
                Color = Color.FromArgb(0xA0, 0x00, 0x00, 0x00),
                Blur = 12,
                OffsetX = 0,
                OffsetY = 4,
            }),
        };

        outer.Child = card;

        // 卡片内部垂直布局
        var cardContent = new DockPanel();

        // ── 图标区域（居中）──
        var iconArea = new Panel
        {
            Height = ThumbIconSize,
        };
        iconArea.Children.Add(new TextBlock
        {
            Text = icon,
            FontSize = 72,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        });
        DockPanel.SetDock(iconArea, Avalonia.Controls.Dock.Top);
        cardContent.Children.Add(iconArea);

        // ── 文字区域（下方）──
        var textArea = new StackPanel
        {
            Margin = new Thickness(ThumbTextPad, ThumbTextPad, ThumbTextPad, ThumbTextPad),
        };

        // 文件名
        var nameLabel = new TextBlock
        {
            Text = name,
            FontSize = 11,
            Foreground = isSelected ? TextBright : TextPrimary,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxLines = 1,
            Height = ThumbNameHeight,
            VerticalAlignment = VerticalAlignment.Center,
            Tag = "NameLabel", // 标记用于重命名时查找
        };
        textArea.Children.Add(nameLabel);

        // 资产类型
        textArea.Children.Add(new TextBlock
        {
            Text = typeLabel,
            FontSize = 9,
            Foreground = badgeColor ?? TextSecondary,
            FontWeight = FontWeight.Bold,
            Height = ThumbTypeHeight,
            VerticalAlignment = VerticalAlignment.Center,
        });

        cardContent.Children.Add(textArea);
        card.Child = cardContent;

        return outer;
    }

    /// <summary>更新缩略图选中状态的视觉效果。</summary>
    internal static void UpdateSelectionVisual(Border card, bool isSelected, bool isDirectory)
    {
        card.Background = isSelected ? BgThumbSelected : (isDirectory ? BgThumbDir : BgThumbFile);
        card.BorderBrush = isSelected ? BorderSelected : Brushes.Transparent;
        card.BorderThickness = new Thickness(isSelected ? 2 : 0);
    }

    /// <summary>更新悬停视觉效果。</summary>
    internal static void UpdateHoverVisual(Border card, bool isHovered, bool isSelected, bool isDirectory)
    {
        if (isHovered && !isSelected)
            card.Background = BgThumbHover;
        else if (!isSelected)
            card.Background = isDirectory ? BgThumbDir : BgThumbFile;
    }
}
