using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// RmlUI 文档组件 Inspector——编辑标志位、排序、视图目标。
/// </summary>
public class RmlUIDocumentInspector : AvaloniaInspectorBase
{
    // RmlUIDocument 组件 TypeId（FNV-1a hash of "RmlUIDocument" = 0x1593AE057DEB826B）
    private const ulong RmlUIDocumentTypeId = 0x1593AE057DEB826B;

    public override string DisplayName => "RmlUI Document";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == RmlUIDocumentTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel { Spacing = 0 };

        // ── SortOrder ──
        content.Children.Add(CreatePropertyRow("Sort Order", CreateNumericInput(0f, 1f)));

        // ── ViewTarget ──
        content.Children.Add(CreatePropertyRow("View Target",
            CreateComboBox(new[] { "Scene", "Game", "Both" })));

        // ── Flags ──
        var flagsPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 12,
            Margin = new Avalonia.Thickness(8, 4),
        };

        flagsPanel.Children.Add(CreateCheckBox("Auto Load", true));
        flagsPanel.Children.Add(CreateCheckBox("Focusable", false));
        flagsPanel.Children.Add(CreateCheckBox("Receives Input", true));

        content.Children.Add(CreatePropertyRow("Flags", flagsPanel));

        return CreateCollapsiblePanel("RmlUI Document", content);
    }

    /// <summary>创建复选框。</summary>
    private static Control CreateCheckBox(string label, bool isChecked)
    {
        var checkBox = new CheckBox
        {
            Content = label,
            IsChecked = isChecked,
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };
        return checkBox;
    }
}
