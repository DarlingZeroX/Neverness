using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 布尔字段编辑器——CheckBox。
/// </summary>
internal static class BoolFieldEditor
{
    /// <summary>创建布尔编辑控件。</summary>
    public static Control Create(FieldDescriptor field, SettingsTable table)
    {
        var checkBox = new CheckBox
        {
            IsChecked = field.Getter?.Invoke(table) as bool? ?? false,
            VerticalAlignment = VerticalAlignment.Center,
        };

        checkBox.IsCheckedChanged += (_, _) =>
        {
            field.Setter?.Invoke(table, checkBox.IsChecked ?? false);
        };

        return checkBox;
    }
}
