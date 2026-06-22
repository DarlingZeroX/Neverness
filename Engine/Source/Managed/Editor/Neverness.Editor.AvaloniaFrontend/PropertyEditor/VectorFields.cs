using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.Inspectors;
using Neverness.Editor.AvaloniaFrontend.Styling;

namespace Neverness.Editor.AvaloniaFrontend.PropertyEditor;

/// <summary>
/// 向量/颜色字段创建器——Vector3、Color 等多轴输入控件。
/// </summary>
public static class VectorFields
{
    // ── 三轴输入 ──

    /// <summary>创建三轴并排输入行（Position / Rotation / Scale），轴标签可点击重置。</summary>
    public static Control CreateVector3(string label, float x, float y, float z,
        float increment = 0.1f, float resetValue = 0f)
    {
        return CreateVector3(label, x, y, z, null, increment, resetValue);
    }

    /// <summary>创建三轴并排输入行，带值变化回调。</summary>
    public static Control CreateVector3(string label, float x, float y, float z,
        Action<float, float, float>? onValueChanged, float increment = 0.1f, float resetValue = 0f)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(8, 4, 8, 4),
            MinHeight = 24,
        };

        var labelText = new TextBlock
        {
            Text = label,
            Width = 80,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = EditorTheme.TextSecondary,
        };
        DockPanel.SetDock(labelText, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelText);

        var axes = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
        };

        DragFloat? inputX = null, inputY = null, inputZ = null;

        void NotifyChanged()
        {
            if (onValueChanged != null && inputX != null && inputY != null && inputZ != null)
                onValueChanged(inputX.Value, inputY.Value, inputZ.Value);
        }

        inputX = CreateAxisInput("X", EditorTheme.AxisX, x, increment, resetValue, NotifyChanged);
        inputY = CreateAxisInput("Y", EditorTheme.AxisY, y, increment, resetValue, NotifyChanged);
        inputZ = CreateAxisInput("Z", EditorTheme.AxisZ, z, increment, resetValue, NotifyChanged);

        axes.Children.Add(inputX);
        axes.Children.Add(inputY);
        axes.Children.Add(inputZ);

        row.Children.Add(axes);
        return row;
    }

    // ── RGBA 颜色输入 ──

    /// <summary>创建 RGBA 颜色行，标签可点击重置为 255。</summary>
    public static Control CreateColor(string label, float r, float g, float b, float a)
    {
        return CreateColor(label, r, g, b, a, null);
    }

    /// <summary>创建 RGBA 颜色行，带值变化回调（值范围 0-255）。</summary>
    public static Control CreateColor(string label, float r, float g, float b, float a,
        Action<float, float, float, float>? onValueChanged)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(8, 4, 8, 4),
            MinHeight = 24,
        };

        var labelText = new TextBlock
        {
            Text = label,
            Width = 80,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = EditorTheme.TextSecondary,
        };
        DockPanel.SetDock(labelText, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelText);

        var axes = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
        };

        DragFloat? inputR = null, inputG = null, inputB = null, inputA = null;

        void NotifyChanged()
        {
            if (onValueChanged != null && inputR != null && inputG != null && inputB != null && inputA != null)
                onValueChanged(inputR.Value, inputG.Value, inputB.Value, inputA.Value);
        }

        inputR = CreateAxisInput("R", EditorTheme.AxisR, r, 1f, 255f, NotifyChanged);
        inputG = CreateAxisInput("G", EditorTheme.AxisG, g, 1f, 255f, NotifyChanged);
        inputB = CreateAxisInput("B", EditorTheme.AxisB, b, 1f, 255f, NotifyChanged);
        inputA = CreateAxisInput("A", EditorTheme.AxisA, a, 1f, 255f, NotifyChanged);

        axes.Children.Add(inputR);
        axes.Children.Add(inputG);
        axes.Children.Add(inputB);
        axes.Children.Add(inputA);

        row.Children.Add(axes);
        return row;
    }

    // ── 内部辅助 ──

    /// <summary>创建单轴输入，带值变化回调。</summary>
    private static DragFloat CreateAxisInput(string axisLabel, SolidColorBrush color, float value, float speed, float resetValue,
        Action? onValueChanged)
    {
        var dragInput = new DragFloat
        {
            Value = value,
            Speed = speed,
            MinWidth = 70,
        };

        // 值变化回调（通过 PropertyChanged 监听）
        if (onValueChanged != null)
            dragInput.PropertyChanged += (_, e) =>
            {
                if (e.Property == DragFloat.ValueProperty)
                    onValueChanged();
            };

        return dragInput;
    }
}
