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
    // Camera 组件 TypeId（与 Native NNCameraComponentData 的 ComponentIdAttribute 一致）
    private const ulong CameraTypeId = 0x54D1B2A64667E32E;

    public override string DisplayName => "Camera";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == CameraTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel { Spacing = 0 };

        content.Children.Add(CreatePropertyRow("Projection", CreateComboBox(new[] { "Perspective", "Orthographic" })));
        content.Children.Add(CreatePropertyRow("FOV", CreateNumericInput(60f, 1f)));
        content.Children.Add(CreatePropertyRow("Near", CreateNumericInput(0.1f, 0.01f)));
        content.Children.Add(CreatePropertyRow("Far", CreateNumericInput(1000f, 10f)));

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
