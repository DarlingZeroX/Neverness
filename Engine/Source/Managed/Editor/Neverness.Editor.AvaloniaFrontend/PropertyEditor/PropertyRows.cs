using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Editor.AvaloniaFrontend.Styling;

namespace Neverness.Editor.AvaloniaFrontend.PropertyEditor;

/// <summary>
/// 属性行创建器——标签 + 编辑器的通用布局。
/// </summary>
public static class PropertyRows
{
    /// <summary>创建属性行（标签 + 编辑器）。</summary>
    public static Control Create(string label, Control editor)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(8, 3, 4, 3),
            MinHeight = 24,
        };

        var labelBlock = new TextBlock
        {
            Text = label,
            Width = 90,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = EditorTheme.TextSecondary,
        };
        DockPanel.SetDock(labelBlock, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelBlock);

        row.Children.Add(editor);
        return row;
    }
}
