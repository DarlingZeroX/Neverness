using Avalonia.Controls;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Transform 组件检查器——编辑 Position/Rotation/Scale。
///
/// 对应 ImGui 版本的 TransformInspector。
/// 使用硬编码 AXAML，不做 PropertyGrid 抽象。
/// </summary>
public class TransformInspector : AvaloniaInspectorBase
{
    // Transform 组件 TypeId（需要与 Native 端一致）
    // TODO: 从 ComponentInspectorRegistry 获取实际 TypeId
    private const ulong TransformTypeId = 1;

    public override string DisplayName => "Transform";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == TransformTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel
        {
            Spacing = 4,
            Margin = new Avalonia.Thickness(0, 4),
        };

        // Position
        content.Children.Add(CreateVector3Input("Position", 0, 0, 0));

        // Rotation (欧拉角)
        content.Children.Add(CreateVector3Input("Rotation", 0, 0, 0));

        // Scale
        content.Children.Add(CreateVector3Input("Scale", 1, 1, 1));

        return CreateCollapsiblePanel("Transform", content);
    }
}
