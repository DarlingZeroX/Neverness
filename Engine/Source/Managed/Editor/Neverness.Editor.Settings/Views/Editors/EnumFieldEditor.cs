using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 枚举字段编辑器——ComboBox。
/// </summary>
internal static class EnumFieldEditor
{
    /// <summary>创建枚举编辑控件。</summary>
    public static Control Create(FieldDescriptor field, SettingsTable table)
    {
        var comboBox = new ComboBox
        {
            MinWidth = EditorTheme.ComboBoxMinWidth,
            HorizontalAlignment = HorizontalAlignment.Left,
        };

        // 填充选项
        var enumValues = field.EnumValues;
        var displayNames = field.EnumDisplayNames;
        if (enumValues != null && displayNames != null)
        {
            foreach (var name in displayNames)
            {
                comboBox.Items.Add(name);
            }

            // 设置当前选中
            var currentValue = field.Getter?.Invoke(table);
            if (currentValue != null)
            {
                var index = Array.IndexOf(enumValues, currentValue);
                if (index >= 0)
                    comboBox.SelectedIndex = index;
            }
        }

        comboBox.SelectionChanged += (_, _) =>
        {
            if (comboBox.SelectedIndex >= 0 && enumValues != null)
            {
                var value = enumValues.GetValue(comboBox.SelectedIndex);
                field.Setter?.Invoke(table, value);
            }
        };

        return comboBox;
    }
}
