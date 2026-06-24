using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Settings.Views.Editors;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views;

/// <summary>
/// 设置面板构建器——根据 ISettingsDescriptor 自动生成字段编辑控件。
/// 负责面板布局、分组、字段行创建。
/// </summary>
internal static class SettingsPanelBuilder
{
    /// <summary>根据描述符创建完整的设置面板。</summary>
    public static Control Create(
        ISettingsDescriptor descriptor,
        SettingsTable table,
        ISettingsService service)
    {
        var panel = new StackPanel
        {
            Spacing = 0,
            Margin = new Thickness(0),
        };

        // 标题
        panel.Children.Add(CreateTitle(descriptor.DisplayName));

        // 按分组组织字段
        var groups = GroupFields(descriptor.Fields);
        var isFirst = true;

        foreach (var (groupName, fields) in groups)
        {
            if (!isFirst)
            {
                panel.Children.Add(CreateSeparator());
            }
            isFirst = false;

            // 分组标题
            if (!string.IsNullOrEmpty(groupName))
            {
                panel.Children.Add(CreateGroupHeader(groupName));
            }

            // 字段编辑器
            foreach (var field in fields)
            {
                if (field.IsHidden) continue;

                var row = CreateFieldRow(field, table);
                if (row != null)
                    panel.Children.Add(row);
            }
        }

        return new ScrollViewer
        {
            Content = panel,
            VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
            HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled,
        };
    }

    /// <summary>创建标题。</summary>
    private static TextBlock CreateTitle(string text)
    {
        return new TextBlock
        {
            Text = text,
            FontSize = EditorTheme.TitleFontSize,
            FontWeight = FontWeight.Bold,
            Foreground = EditorTheme.TextPrimaryBrush,
            Margin = new Thickness(0, 0, 0, 16),
        };
    }

    /// <summary>创建分隔线。</summary>
    private static Border CreateSeparator()
    {
        return new Border
        {
            Height = 1,
            Background = EditorTheme.SeparatorBrush,
            Margin = new Thickness(0, 8, 0, 8),
        };
    }

    /// <summary>创建分组标题。</summary>
    private static TextBlock CreateGroupHeader(string text)
    {
        return new TextBlock
        {
            Text = text,
            FontSize = EditorTheme.GroupFontSize,
            FontWeight = FontWeight.Bold,
            Foreground = EditorTheme.TextSecondaryBrush,
            Margin = new Thickness(0, 4, 0, 8),
        };
    }

    /// <summary>按分组组织字段。</summary>
    private static List<(string? GroupName, List<FieldDescriptor> Fields)> GroupFields(
        IReadOnlyList<FieldDescriptor> fields)
    {
        var result = new List<(string?, List<FieldDescriptor>)>();
        var groupMap = new Dictionary<string, List<FieldDescriptor>>(StringComparer.Ordinal);

        foreach (var field in fields.OrderBy(f => f.Order))
        {
            var key = field.Group ?? "";
            if (!groupMap.TryGetValue(key, out var list))
            {
                list = new List<FieldDescriptor>();
                groupMap[key] = list;
                result.Add((field.Group, list));
            }
            list.Add(field);
        }

        return result;
    }

    /// <summary>创建字段行（标签 + 编辑器，带悬停效果）。</summary>
    private static Control? CreateFieldRow(FieldDescriptor field, SettingsTable table)
    {
        var editor = FieldEditorFactory.Create(field, table);
        if (editor == null) return null;

        // 标签
        var label = new TextBlock
        {
            Text = field.DisplayName,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = EditorTheme.TextPrimaryBrush,
            FontSize = EditorTheme.LabelFontSize,
            MinWidth = EditorTheme.LabelMinWidth,
            Margin = new Thickness(0, 0, 12, 0),
        };

        // 描述提示
        if (!string.IsNullOrEmpty(field.Description))
            ToolTip.SetTip(label, field.Description);

        // 只读标记
        if (field.IsReadOnly)
            editor.IsEnabled = false;

        // 布局：标签在左，编辑器在右
        var rowContent = new DockPanel
        {
            Margin = new Thickness(8, 0),
            MinHeight = EditorTheme.RowMinHeight,
        };

        DockPanel.SetDock(label, Dock.Left);
        rowContent.Children.Add(label);
        rowContent.Children.Add(editor);

        // 带悬停效果的容器
        var rowBorder = new Border
        {
            Child = rowContent,
            CornerRadius = new CornerRadius(4),
            Margin = new Thickness(0, 2),
            Padding = new Thickness(4),
        };

        rowBorder.PointerEntered += (_, _) =>
            rowBorder.Background = new SolidColorBrush(Color.Parse("#1AFFFFFF"));
        rowBorder.PointerExited += (_, _) =>
            rowBorder.Background = Brushes.Transparent;

        return rowBorder;
    }
}
