using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 降级字段编辑器——不支持的字段类型显示提示。
/// </summary>
internal static class FallbackFieldEditor
{
    /// <summary>创建降级编辑控件。</summary>
    public static Control Create(FieldDescriptor field)
    {
        return new TextBlock
        {
            Text = $"[不支持的字段类型: {field.FieldType}]",
            Foreground = EditorTheme.WarningBrush,
            FontStyle = FontStyle.Italic,
            VerticalAlignment = VerticalAlignment.Center,
        };
    }
}
