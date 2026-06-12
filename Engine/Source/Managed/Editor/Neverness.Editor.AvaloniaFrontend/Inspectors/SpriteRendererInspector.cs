using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// SpriteRenderer 组件检查器——编辑纹理、颜色、UV 等。
///
/// 对应 ImGui 版本的 SpriteRendererInspector。
/// </summary>
public class SpriteRendererInspector : AvaloniaInspectorBase
{
    // SpriteRenderer 组件 TypeId
    // TODO: 从 ComponentInspectorRegistry 获取实际 TypeId
    private const ulong SpriteRendererTypeId = 3;

    public override string DisplayName => "SpriteRenderer";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == SpriteRendererTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel
        {
            Spacing = 4,
            Margin = new Avalonia.Thickness(0, 4),
        };

        // 纹理引用（拖拽区域）
        var textureArea = new Border
        {
            Width = 100,
            Height = 100,
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            BorderBrush = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            BorderThickness = new Avalonia.Thickness(1),
            CornerRadius = new Avalonia.CornerRadius(4),
            Child = new TextBlock
            {
                Text = "Drop\nTexture",
                FontSize = 10,
                Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
                HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Center,
                VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
                TextAlignment = Avalonia.Media.TextAlignment.Center,
            },
        };
        content.Children.Add(CreatePropertyRow("Texture", textureArea));

        // 颜色
        var colorPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
        };

        var rInput = CreateNumericInput(255, 1);
        rInput.Maximum = 255;
        rInput.Minimum = 0;
        var gInput = CreateNumericInput(255, 1);
        gInput.Maximum = 255;
        gInput.Minimum = 0;
        var bInput = CreateNumericInput(255, 1);
        bInput.Maximum = 255;
        bInput.Minimum = 0;
        var aInput = CreateNumericInput(255, 1);
        aInput.Maximum = 255;
        aInput.Minimum = 0;

        var rLabel = new TextBlock { Text = "R", FontSize = 10, Foreground = new SolidColorBrush(Color.Parse("#FFF44336")), VerticalAlignment = VerticalAlignment.Center };
        var gLabel = new TextBlock { Text = "G", FontSize = 10, Foreground = new SolidColorBrush(Color.Parse("#FF4CAF50")), VerticalAlignment = VerticalAlignment.Center };
        var bLabel = new TextBlock { Text = "B", FontSize = 10, Foreground = new SolidColorBrush(Color.Parse("#FF2196F3")), VerticalAlignment = VerticalAlignment.Center };
        var aLabel = new TextBlock { Text = "A", FontSize = 10, Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")), VerticalAlignment = VerticalAlignment.Center };

        colorPanel.Children.Add(rLabel);
        colorPanel.Children.Add(rInput);
        colorPanel.Children.Add(gLabel);
        colorPanel.Children.Add(gInput);
        colorPanel.Children.Add(bLabel);
        colorPanel.Children.Add(bInput);
        colorPanel.Children.Add(aLabel);
        colorPanel.Children.Add(aInput);

        content.Children.Add(CreatePropertyRow("Color", colorPanel));

        // Layer
        var layerInput = new NumericUpDown
        {
            Value = 0,
            Increment = 1,
            Minimum = 0,
            Maximum = 255,
            Width = 80,
            Height = 24,
        };
        content.Children.Add(CreatePropertyRow("Layer", layerInput));

        // Sort Order
        var sortOrderInput = new NumericUpDown
        {
            Value = 0,
            Increment = 1,
            Width = 80,
            Height = 24,
        };
        content.Children.Add(CreatePropertyRow("Sort Order", sortOrderInput));

        // Blend Mode
        var blendCombo = new ComboBox
        {
            Width = 120,
            Height = 24,
            ItemsSource = new[] { "Alpha", "Additive", "Multiply" },
            SelectedIndex = 0,
        };
        content.Children.Add(CreatePropertyRow("Blend Mode", blendCombo));

        return CreateCollapsiblePanel("SpriteRenderer", content);
    }
}
