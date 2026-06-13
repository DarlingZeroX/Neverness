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
    // Transform 组件 TypeId（与 Native NNTransformData 的 ComponentIdAttribute 一致）
    private const ulong TransformTypeId = 0xC1FFF4F356DFB2FB;

    public override string DisplayName => "Transform";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == TransformTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var content = new StackPanel { Spacing = 0 };

        // ImGui 风格：一行 Label + X/Y/Z 并排输入
        // 点击 X/Y/Z 按钮重置：Position/Rotation 归零，Scale 置 1
        content.Children.Add(CreateVector3Row("Position", 0, 0, 0, 0.1f, resetValue: 0f));
        content.Children.Add(CreateVector3Row("Rotation", 0, 0, 0, 1f, resetValue: 0f));
        content.Children.Add(CreateVector3Row("Scale", 1, 1, 1, 0.01f, resetValue: 1f));

        return CreateCollapsiblePanel("Transform", content);
    }
}
