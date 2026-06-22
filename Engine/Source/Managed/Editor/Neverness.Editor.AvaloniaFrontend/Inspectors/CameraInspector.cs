using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.Styling;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Camera 组件检查器——编辑投影类型、裁剪面、FOV、正交大小等。
///
/// 对应 ImGui 版本的 CameraInspector。
/// 实现细节：
/// - 通过 IInspectorService.GetEntityById 获取 IEntity
/// - 通过 IEntity.Get&lt;CameraComponent&gt;() 读写组件数据
/// - DragFloat.PropertyChanged 监听用户编辑，写回 ECS
/// - 投影类型切换时动态显示/隐藏相关字段
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
        // 从 ECS 获取实体和组件当前值
        var entity = GetEntityById((int)entityHandle);
        bool isOrthographic = false;
        float fov = MathF.PI / 4f;           // 45 度
        float aspectRatio = 16f / 9f;
        float nearPlane = 0.1f;
        float farPlane = 1000f;
        float orthoSize = 10f;

        if (entity != null && entity.IsValid && entity.Has<CameraComponent>())
        {
            ref var camera = ref entity.Get<CameraComponent>();
            isOrthographic = camera.IsOrthographic;
            fov = camera.FieldOfView;
            aspectRatio = camera.AspectRatio;
            nearPlane = camera.NearPlane;
            farPlane = camera.FarPlane;
            orthoSize = camera.OrthographicSize;
        }

        var content = new StackPanel { Spacing = 0 };

        // ── Projection Type ──
        content.Children.Add(CreateProjectionTypeRow(isOrthographic, value =>
        {
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
            ref var c = ref ent.Get<CameraComponent>();
            c.IsOrthographic = value;
        }));

        // ── Clipping Planes ──
        content.Children.Add(CreateEditableFloatRow("Near Plane", nearPlane,
            increment: 0.01f, min: 0.01f, max: 10000f,
            onValueChanged: v =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
                ref var c = ref ent.Get<CameraComponent>();
                c.NearPlane = v;
            }));

        content.Children.Add(CreateEditableFloatRow("Far Plane", farPlane,
            increment: 1f, min: 1f, max: 100000f,
            onValueChanged: v =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
                ref var c = ref ent.Get<CameraComponent>();
                c.FarPlane = v;
            }));

        // ── Perspective 专属字段 ──
        var perspectivePanel = new StackPanel { Spacing = 0, IsVisible = !isOrthographic };

        var fovRow = CreateEditableFloatRow("Field of View", fov,
            increment: 1f, min: 1f, max: 179f,
            onValueChanged: v =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
                ref var c = ref ent.Get<CameraComponent>();
                c.FieldOfView = v;
            });
        perspectivePanel.Children.Add(fovRow);

        var aspectRow = CreateEditableFloatRow("Aspect Ratio", aspectRatio,
            increment: 0.01f, min: 0.01f, max: 10f,
            onValueChanged: v =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
                ref var c = ref ent.Get<CameraComponent>();
                c.AspectRatio = v;
            });
        perspectivePanel.Children.Add(aspectRow);

        content.Children.Add(perspectivePanel);

        // ── Orthographic 专属字段 ──
        var orthoPanel = new StackPanel { Spacing = 0, IsVisible = isOrthographic };

        var orthoRow = CreateEditableFloatRow("Ortho Size", orthoSize,
            increment: 0.1f, min: 0.1f, max: 10000f,
            onValueChanged: v =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<CameraComponent>()) return;
                ref var c = ref ent.Get<CameraComponent>();
                c.OrthographicSize = v;
            });
        orthoPanel.Children.Add(orthoRow);

        content.Children.Add(orthoPanel);

        return CreateCollapsiblePanel("Camera", content);
    }

    // ── 投影类型选择行 ──

    /// <summary>创建投影类型选择行（Perspective / Orthographic）。</summary>
    private Control CreateProjectionTypeRow(bool isOrthographic, Action<bool> onValueChanged)
    {
        var row = new DockPanel
        {
            Margin = new Thickness(8, 4, 8, 4),
            MinHeight = 24,
        };

        var labelText = new TextBlock
        {
            Text = "Projection",
            Width = 80,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Foreground = EditorTheme.TextSecondary,
        };
        DockPanel.SetDock(labelText, Avalonia.Controls.Dock.Left);
        row.Children.Add(labelText);

        var comboBox = new ComboBox
        {
            ItemsSource = new[] { "Perspective", "Orthographic" },
            SelectedIndex = isOrthographic ? 1 : 0,
            MinWidth = 120,
            Height = 22,
            HorizontalAlignment = HorizontalAlignment.Stretch,
        };

        comboBox.SelectionChanged += (_, e) =>
        {
            if (comboBox.SelectedIndex >= 0)
            {
                onValueChanged(comboBox.SelectedIndex == 1);
            }
        };

        row.Children.Add(comboBox);
        return row;
    }

    // ── 可编辑浮点输入行 ──

    /// <summary>
    /// 创建可编辑的浮点输入行，DragFloat 值变化时回调 onValueChanged。
    /// </summary>
    private Control CreateEditableFloatRow(string label, float initialValue,
        float increment, float min, float max, Action<float> onValueChanged)
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

        var dragInput = new DragFloat
        {
            Value = initialValue,
            Speed = increment,
            Min = min,
            Max = max,
            MinWidth = 120,
        };

        dragInput.PropertyChanged += (_, e) =>
        {
            if (e.Property == DragFloat.ValueProperty)
            {
                onValueChanged(dragInput.Value);
            }
        };

        row.Children.Add(dragInput);
        return row;
    }

}
