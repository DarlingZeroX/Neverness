using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 浮点数字段编辑器——NumericUpDown（有范围时加 Slider）。
/// </summary>
internal static class FloatFieldEditor
{
    /// <summary>创建浮点数编辑控件。</summary>
    public static Control Create(FieldDescriptor field, SettingsTable table)
    {
        var value = field.Getter?.Invoke(table) as float? ?? 0f;

        if (field.HasRange)
        {
            return CreateSliderWithNumeric(field, table, value);
        }

        return CreateNumericOnly(field, table, value);
    }

    /// <summary>创建 Slider + NumericUpDown 组合。</summary>
    private static Control CreateSliderWithNumeric(FieldDescriptor field, SettingsTable table, float value)
    {
        var grid = new Grid { ColumnDefinitions = new ColumnDefinitions("*,8,80") };

        var slider = new Slider
        {
            Minimum = field.Min,
            Maximum = field.Max,
            Value = value,
            TickFrequency = field.Step > 0 ? field.Step : 0.01,
            VerticalAlignment = VerticalAlignment.Center,
        };

        var numeric = new NumericUpDown
        {
            Value = (decimal)value,
            Minimum = (decimal)field.Min,
            Maximum = (decimal)field.Max,
            Increment = field.Step > 0 ? (decimal)field.Step : 0.01m,
            FormatString = "F2",
            Width = EditorTheme.NumericWidth,
        };

        // 双向同步
        slider.PropertyChanged += (_, e) =>
        {
            if (e.Property.Name == nameof(Slider.Value))
            {
                var v = (float)slider.Value;
                numeric.Value = (decimal)v;
                field.Setter?.Invoke(table, v);
            }
        };

        numeric.ValueChanged += (_, e) =>
        {
            if (e.NewValue.HasValue)
            {
                var v = (float)e.NewValue.Value;
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
    private static Control CreateNumericOnly(FieldDescriptor field, SettingsTable table, float value)
    {
        var numeric = new NumericUpDown
        {
            Value = (decimal)value,
            Increment = 0.01m,
            FormatString = "F2",
            MinWidth = 120,
            HorizontalAlignment = HorizontalAlignment.Left,
        };

        numeric.ValueChanged += (_, e) =>
        {
            if (e.NewValue.HasValue)
                field.Setter?.Invoke(table, (float)e.NewValue.Value);
        };

        return numeric;
    }
}
