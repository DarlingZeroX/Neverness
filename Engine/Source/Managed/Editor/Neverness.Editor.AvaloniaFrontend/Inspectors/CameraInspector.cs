using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Camera 组件检查器——编辑投影类型、裁剪面、FOV 等。
///
/// 对应 ImGui 版本的 CameraInspector。
/// </summary>
public class CameraInspector : AvaloniaInspectorBase
{
    // Camera 组件 TypeId
    // TODO: 从 ComponentInspectorRegistry 获取实际 TypeId
    private const ulong CameraTypeId = 2;

    public override string DisplayName => "Camera";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == CameraTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel
        {
            Spacing = 4,
            Margin = new Avalonia.Thickness(0, 4),
        };

        // 投影类型
        var projectionCombo = new ComboBox
        {
            Width = 120,
            Height = 24,
            ItemsSource = new[] { "Perspective", "Orthographic" },
            SelectedIndex = 0,
        };
        content.Children.Add(CreatePropertyRow("Projection", projectionCombo));

        // FOV
        var fovInput = CreateNumericInput(60f, 1f);
        content.Children.Add(CreatePropertyRow("FOV", fovInput));

        // 近裁剪面
        var nearInput = CreateNumericInput(0.1f, 0.01f);
        content.Children.Add(CreatePropertyRow("Near", nearInput));

        // 远裁剪面
        var farInput = CreateNumericInput(1000f, 10f);
        content.Children.Add(CreatePropertyRow("Far", farInput));

        // 宽高比（只读）
        var aspectLabel = new TextBlock
        {
            Text = "16:9",
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
        };
        content.Children.Add(CreatePropertyRow("Aspect", aspectLabel));

        return CreateCollapsiblePanel("Camera", content);
    }
}
