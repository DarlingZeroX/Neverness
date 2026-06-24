using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 整数字段编辑器——NumericUpDown（有范围时加 Slider）。
/// </summary>
internal static class IntFieldEditor
{
    /// <summary>创建整数编辑控件。</summary>
    public static Control Create(FieldDescriptor field, SettingsTable table)
    {
        var value = field.Getter?.Invoke(table) as int? ?? 0;

        if (field.HasRange)
        {
            return CreateSliderWithNumeric(field, table, value);
        }

        return CreateNumericOnly(field, table, value);
    }

    /// <summary>创建 Slider + NumericUpDown 组合。</summary>
    private static Control CreateSliderWithNumeric(FieldDescriptor field, SettingsTable table, int value)
    {
        var grid = new Grid { ColumnDefinitions = new ColumnDefinitions("*,8,80") };

        var slider = new Slider
        {
            Minimum = field.Min,
            Maximum = field.Max,
            Value = value,
            TickFrequency = field.Step > 0 ? field.Step : 1,
            VerticalAlignment = VerticalAlignment.Center,
        };

        var numeric = new NumericUpDown
        {
            Value = value,
            Minimum = (int)field.Min,
            Maximum = (int)field.Max,
            Increment = field.Step > 0 ? (decimal)field.Step : 1,
            FormatString = "F0",
            Width = EditorTheme.NumericWidth,
        };

        // 双向同步
        slider.PropertyChanged += (_, e) =>
        {
            if (e.Property.Name == nameof(Slider.Value))
            {
                var v = (int)slider.Value;
                numeric.Value = v;
                field.Setter?.Invoke(table, v);
            }
        };

        numeric.ValueChanged += (_, e) =>
        {
            if (e.NewValue.HasValue)
            {
                var v = (int)e.NewValue.Value;
                slider.Value = v;
                field.Setter?.Invoke(table, v);
            }
        };

        grid.Children.Add(slider);
        Grid.SetColumn(numeric, 2);
        grid.Children.Add(numeric);

        return grid;
    }

    /// <summary>创建纯 NumericUpDown。</summary>
    private static Control CreateNumericOnly(FieldDescriptor field, SettingsTable table, int value)
    {
        var numeric = new NumericUpDown
        {
            Value = value,
            Increment = 1,
            FormatString = "F0",
            MinWidth = 120,
            HorizontalAlignment = HorizontalAlignment.Left,
        };

        numeric.ValueChanged += (_, e) =>
        {
            if (e.NewValue.HasValue)
                field.Setter?.Invoke(table, (int)e.NewValue.Value);
        };

        return numeric;
    }
}
