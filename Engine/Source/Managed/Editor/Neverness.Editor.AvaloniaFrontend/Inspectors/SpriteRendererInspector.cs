using Avalonia;
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
    // SpriteRenderer 组件 TypeId（与 Native NNSpriteRendererComponentData 的 ComponentIdAttribute 一致）
    private const ulong SpriteRendererTypeId = 0x51387BA3968C343B;

    public override string DisplayName => "SpriteRenderer";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == SpriteRendererTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel { Spacing = 0 };

        // 纹理
        var textureArea = new Border
        {
            Width = 80,
            Height = 80,
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            BorderBrush = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(4),
            Child = new TextBlock
            {
                Text = "Drop\nTexture",
                FontSize = 10,
                Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                TextAlignment = TextAlignment.Center,
            },
        };
        content.Children.Add(CreatePropertyRow("Texture", textureArea));

        // 颜色 RGBA（与 Transform 的 XYZ 同风格）
        content.Children.Add(CreateColorRow("Color", 255, 255, 255, 255));

        content.Children.Add(CreatePropertyRow("Layer", CreateNumericInput(0, 1)));
        content.Children.Add(CreatePropertyRow("Sort Order", CreateNumericInput(0, 1)));
        content.Children.Add(CreatePropertyRow("Blend Mode", CreateComboBox(new[] { "Alpha", "Additive", "Multiply" })));

        return CreateCollapsiblePanel("SpriteRenderer", content);
    }
}
