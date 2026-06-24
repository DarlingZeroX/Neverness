using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 字符串字段编辑器——TextBox。
/// </summary>
internal static class StringFieldEditor
{
    /// <summary>创建字符串编辑控件。</summary>
    public static Control Create(FieldDescriptor field, SettingsTable table)
    {
        var textBox = new TextBox
        {
            Text = field.Getter?.Invoke(table) as string ?? "",
            MinWidth = EditorTheme.TextBoxMinWidth,
            HorizontalAlignment = HorizontalAlignment.Stretch,
        };

        textBox.LostFocus += (_, _) =>
        {
            field.Setter?.Invoke(table, textBox.Text ?? "");
        };

        return textBox;
    }
}
