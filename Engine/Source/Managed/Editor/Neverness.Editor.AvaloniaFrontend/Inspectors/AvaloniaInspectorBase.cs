using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Layout;
using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Avalonia Inspector 基类——组件检查器的抽象基类。
/// </summary>
public abstract class AvaloniaInspectorBase
{
    // ── 颜色常量 ──
    private static readonly SolidColorBrush ColorBg = new(Color.Parse("#FF1E1E1E"));
    private static readonly SolidColorBrush ColorHeaderBg = new(Color.Parse("#FF2D2D30"));
    private static readonly SolidColorBrush ColorRowBg = new(Color.Parse("#FF252526"));
    private static readonly SolidColorBrush ColorLabel = new(Color.Parse("#FF999999"));
    private static readonly SolidColorBrush ColorText = new(Color.Parse("#FFCCCCCC"));
    private static readonly SolidColorBrush ColorAxisX = new(Color.Parse("#FFF44336"));
    private static readonly SolidColorBrush ColorAxisY = new(Color.Parse("#FF4CAF50"));
    private static readonly SolidColorBrush ColorAxisZ = new(Color.Parse("#FF2196F3"));
    private static readonly SolidColorBrush ColorR = new(Color.Parse("#FFF44336"));
    private static readonly SolidColorBrush ColorG = new(Color.Parse("#FF4CAF50"));
    private static readonly SolidColorBrush ColorB = new(Color.Parse("#FF2196F3"));
    private static readonly SolidColorBrush ColorA = new(Color.Parse("#FFCCCCCC"));

    public abstract string DisplayName { get; }
    public abstract bool CanInspect(ulong typeId);
    public abstract Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId);

    // ── 可折叠组件面板 ──

    protected static Control CreateCollapsiblePanel(string title, Control content, bool isExpanded = true)
    {
        var root = new DockPanel { Margin = new Thickness(0, 0, 0, 2) };

        // 头部
        var header = new DockPanel
        {
            Background = ColorHeaderBg,
            Height = 26,
            Cursor = new Avalonia.Input.Cursor(Avalonia.Input.StandardCursorType.Hand),
        };

        var expandIcon = new TextBlock
        {
            Text = isExpanded ? "▼" : "▶",
            Width = 20,
            FontSize = 9,
            VerticalAlignment = VerticalAlignment.Center,
            HorizontalAlignment = HorizontalAlignment.Center,
            Foreground = ColorLabel,
        };
        DockPanel.SetDock(expandIcon, Avalonia.Controls.Dock.Left);
        header.Children.Add(expandIcon);

        var titleBlock = new TextBlock
        {
            Text = title,
            FontSize = 12,
            FontWeight = FontWeight.Bold,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(4, 0),
            Foreground = ColorText,
        };
        header.Children.Add(titleBlock);

        DockPanel.SetDock(header, Avalonia.Controls.Dock.Top);
        root.Children.Add(header);

        // 内容区域
        content.IsVisible = isExpanded;
        root.Children.Add(content);

        // 点击展开/折叠
        header.PointerPressed += (_, _) =>
        {
            content.IsVisible = !content.IsVisible;
            expandIcon.Text = content.IsVisible ? "▼" : "▶";
        };

        return root;
    }

    // ── 属性行 ──

    protected static Control CreatePropertyRow(string label, Control editor)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(8, 3, 4, 3),
            MinHeight = 24,
        };

        var labelBlock = new TextBlock
        {
            Text = label,
            Width = 90,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = ColorLabel,
        };
        DockPanel.SetDock(labelBlock, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelBlock);

        row.Children.Add(editor);
        return row;
    }

    // ── 三轴输入（ImGui 风格：一行 Label + X/Y/Z 并排）──

    /// <summary>创建三轴并排输入行（Position / Rotation / Scale），轴标签可点击重置。</summary>
    protected static Control CreateVector3Row(string label, float x, float y, float z,
        float increment = 0.1f, float resetValue = 0f)
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
            Foreground = ColorLabel,
        };
        DockPanel.SetDock(labelText, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelText);

        var axes = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
        };

        axes.Children.Add(CreateAxisInput("X", ColorAxisX, x, increment, resetValue));
        axes.Children.Add(CreateAxisInput("Y", ColorAxisY, y, increment, resetValue));
        axes.Children.Add(CreateAxisInput("Z", ColorAxisZ, z, increment, resetValue));

        row.Children.Add(axes);
        return row;
    }

    /// <summary>创建 RGBA 颜色行，标签可点击重置为 255。</summary>
    protected static Control CreateColorRow(string label, float r, float g, float b, float a)
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
            Foreground = ColorLabel,
        };
        DockPanel.SetDock(labelText, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelText);

        var axes = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
        };

        axes.Children.Add(CreateAxisInput("R", ColorR, r, 1f, 255f));
        axes.Children.Add(CreateAxisInput("G", ColorG, g, 1f, 255f));
        axes.Children.Add(CreateAxisInput("B", ColorB, b, 1f, 255f));
        axes.Children.Add(CreateAxisInput("A", ColorA, a, 1f, 255f));

        row.Children.Add(axes);
        return row;
    }

    /// <summary>创建单轴输入（轴标签按钮 + DragFloat），点击轴标签重置数值。</summary>
    private static Control CreateAxisInput(string axisLabel, SolidColorBrush color, float value, float speed, float resetValue)
    {
        var panel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
        };

        // 轴标签按钮：毛玻璃背景，点击重置数值
        var dragInput = new DragFloat
        {
            Value = value,
            Speed = speed,
            MinWidth = 70,
        };

        var labelBtn = new Button
        {
            Content = axisLabel,
            FontSize = 11,
            FontWeight = FontWeight.Bold,
            Width = 20,
            Height = 22,
            Padding = new Thickness(0),
            Background = new SolidColorBrush(Color.FromArgb(180, 55, 55, 60)),
            BorderThickness = new Thickness(0),
            CornerRadius = new CornerRadius(3),
            Foreground = color,
            HorizontalContentAlignment = Avalonia.Layout.HorizontalAlignment.Center,
            VerticalContentAlignment = Avalonia.Layout.VerticalAlignment.Center,
            Cursor = new Avalonia.Input.Cursor(Avalonia.Input.StandardCursorType.Hand),
        };
        labelBtn.Click += (_, _) => dragInput.Value = resetValue;
        panel.Children.Add(labelBtn);

        panel.Children.Add(dragInput);

        return panel;
    }

    // ── Slider 行（给 SpriteRenderer 等用）──

    /// <summary>创建单个 Slider 行（标签 + 轴标签 + Slider + 数值显示）。</summary>
    protected static Control CreateSliderRow(string label, string axisLabel, SolidColorBrush axisColor,
        float value, float min = -10000f, float max = 10000f, float tickFreq = 1f)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(12, 2, 8, 2),
            Height = 22,
        };

        var axis = new TextBlock
        {
            Text = axisLabel,
            Width = 16,
            FontSize = 11,
            FontWeight = FontWeight.Bold,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = axisColor,
        };
        DockPanel.SetDock(axis, Avalonia.Controls.Dock.Left);
        row.Children.Add(axis);

        var valueText = new TextBlock
        {
            Text = value.ToString("F2"),
            Width = 50,
            FontSize = 11,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = ColorText,
            TextAlignment = TextAlignment.Right,
            Margin = new Thickness(4, 0, 0, 0),
        };
        DockPanel.SetDock(valueText, Avalonia.Controls.Dock.Right);
        row.Children.Add(valueText);

        var slider = new Slider
        {
            Minimum = min,
            Maximum = max,
            Value = value,
            TickFrequency = tickFreq,
            IsSnapToTickEnabled = false,
            VerticalAlignment = VerticalAlignment.Center,
            Height = 16,
        };
        slider.PropertyChanged += (_, e) =>
        {
            if (e.Property.Name == nameof(Slider.Value))
                valueText.Text = slider.Value.ToString("F2");
        };
        row.Children.Add(slider);

        return row;
    }

    // ── 数值输入框（保留给 Camera 等需要精确输入的场景）──

    protected static NumericUpDown CreateNumericInput(float value, float increment = 0.1f)
    {
        return new NumericUpDown
        {
            Value = (decimal)value,
            Increment = (decimal)increment,
            FormatString = "F2",
            MinWidth = 70,
            Height = 22,
            HorizontalAlignment = HorizontalAlignment.Stretch,
        };
    }

    // ── ComboBox ──

    protected static ComboBox CreateComboBox(string[] items, int selectedIndex = 0)
    {
        return new ComboBox
        {
            ItemsSource = items,
            SelectedIndex = selectedIndex,
            MinWidth = 100,
            Height = 22,
        };
    }
}
