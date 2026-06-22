using Avalonia.Controls;

namespace Neverness.Editor.AvaloniaFrontend.PropertyEditor;

/// <summary>
/// 数值输入字段创建器——NumericUpDown 控件工厂。
/// </summary>
public static class NumericFields
{
    // ── 浮点数输入 ──

    /// <summary>创建浮点数输入框。</summary>
    public static NumericUpDown CreateFloat(float value, float increment = 0.1f)
    {
        return CreateFloat(value, increment, null);
    }

    /// <summary>创建浮点数输入框，带值变化回调。</summary>
    public static NumericUpDown CreateFloat(float value, float increment, Action<float>? onValueChanged)
    {
        var input = new NumericUpDown
        {
            Value = (decimal)value,
            Increment = (decimal)increment,
            FormatString = "F2",
            MinWidth = 70,
            Height = 22,
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Stretch,
        };

        if (onValueChanged != null)
            input.ValueChanged += (_, e) => onValueChanged((float)(e.NewValue ?? 0));

        return input;
    }

    // ── 整数输入 ──

    /// <summary>创建无符号整数输入框。</summary>
    public static NumericUpDown CreateUInt(uint value, uint increment = 1)
    {
        return CreateUInt(value, increment, null);
    }

    /// <summary>创建无符号整数输入框，带值变化回调。</summary>
    public static NumericUpDown CreateUInt(uint value, uint increment, Action<uint>? onValueChanged)
    {
        var input = new NumericUpDown
        {
            Value = (decimal)value,
            Increment = (decimal)increment,
            FormatString = "F0",  // 整数格式，不显示小数
            Minimum = 0,          // uint 最小值
            MinWidth = 70,
            Height = 22,
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Stretch,
            ParsingNumberStyle = System.Globalization.NumberStyles.Integer,
        };

        if (onValueChanged != null)
            input.ValueChanged += (_, e) => onValueChanged((uint)Math.Max(0, (int)(e.NewValue ?? 0)));

        return input;
    }

    // ── 下拉框 ──

    /// <summary>创建下拉框。</summary>
    public static ComboBox CreateCombo(string[] items, int selectedIndex = 0)
    {
        return CreateCombo(items, selectedIndex, null);
    }

    /// <summary>创建下拉框，带选中变化回调。</summary>
    public static ComboBox CreateCombo(string[] items, int selectedIndex, Action<int>? onSelectionChanged)
    {
        var comboBox = new ComboBox
        {
            ItemsSource = items,
            SelectedIndex = selectedIndex,
            MinWidth = 100,
            Height = 22,
        };

        if (onSelectionChanged != null)
            comboBox.SelectionChanged += (_, e) => onSelectionChanged(comboBox.SelectedIndex);

        return comboBox;
    }
}
