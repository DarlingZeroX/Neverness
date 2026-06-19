using System.Numerics;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Transform 组件检查器——编辑 Position/Rotation/Scale。
///
/// 对应 ImGui 版本的 TransformInspector。
/// 使用硬编码 AXAML，不做 PropertyGrid 抽象。
/// 旋转以欧拉角（度）显示/编辑，内部存储为四元数。
///
/// 实现细节：
/// - 通过 IInspectorService.GetEntityById 获取 IEntity
/// - 通过 IEntity.Get&lt;TransformComponent&gt;() 读写组件数据
/// - DragFloat.PropertyChanged 监听用户编辑，写回 ECS
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
        // 从 ECS 获取实体和组件当前值
        var entity = GetEntityById((int)entityHandle);
        Vector3 position = Vector3.Zero;
        Vector3 rotationEuler = Vector3.Zero;
        Vector3 scale = Vector3.One;

        if (entity != null && entity.IsValid && entity.Has<TransformComponent>())
        {
            ref var transform = ref entity.Get<TransformComponent>();
            position = transform.Position;
            rotationEuler = QuaternionToEuler(transform.Rotation);
            scale = transform.Scale;
        }

        var content = new StackPanel { Spacing = 0 };

        // ── Position ──
        content.Children.Add(CreateEditableVector3Row("Position", position,
            increment: 0.1f, resetValue: 0f,
            onValueChanged: newPos =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<TransformComponent>()) return;
                ref var t = ref ent.Get<TransformComponent>();
                t.Position = newPos;
            }));

        // ── Rotation（以欧拉角显示/编辑，内部四元数） ──
        content.Children.Add(CreateEditableVector3Row("Rotation", rotationEuler,
            increment: 1f, resetValue: 0f,
            onValueChanged: eulerDeg =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<TransformComponent>()) return;
                ref var t = ref ent.Get<TransformComponent>();
                t.Rotation = EulerToQuaternion(eulerDeg);
            }));

        // ── Scale ──
        content.Children.Add(CreateEditableVector3Row("Scale", scale,
            increment: 0.01f, resetValue: 1f,
            onValueChanged: newScale =>
            {
                var ent = GetEntityById((int)entityHandle);
                if (ent == null || !ent.IsValid || !ent.Has<TransformComponent>()) return;
                ref var t = ref ent.Get<TransformComponent>();
                t.Scale = newScale;
            }));

        return CreateCollapsiblePanel("Transform", content);
    }

    // ── 可编辑三轴输入行 ──

    /// <summary>
    /// 创建可编辑的三轴输入行，DragFloat 值变化时回调 onValueChanged。
    /// 与基类 CreateVector3Row 不同，此方法保持 DragFloat 引用用于数据绑定。
    /// </summary>
    private static Control CreateEditableVector3Row(string label, Vector3 initialValue,
        float increment, float resetValue, Action<Vector3> onValueChanged)
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

        // 创建三轴（轴标签按钮 + DragFloat），保持 DragFloat 引用
        var (panelX, dragX) = CreateAxisWithButton("X", ColorAxisX, initialValue.X, increment, resetValue);
        var (panelY, dragY) = CreateAxisWithButton("Y", ColorAxisY, initialValue.Y, increment, resetValue);
        var (panelZ, dragZ) = CreateAxisWithButton("Z", ColorAxisZ, initialValue.Z, increment, resetValue);

        // 监听值变化，聚合为 Vector3 回调
        void NotifyChange()
        {
            onValueChanged(new Vector3(dragX.Value, dragY.Value, dragZ.Value));
        }

        dragX.PropertyChanged += (_, e) => { if (e.Property == DragFloat.ValueProperty) NotifyChange(); };
        dragY.PropertyChanged += (_, e) => { if (e.Property == DragFloat.ValueProperty) NotifyChange(); };
        dragZ.PropertyChanged += (_, e) => { if (e.Property == DragFloat.ValueProperty) NotifyChange(); };

        axes.Children.Add(panelX);
        axes.Children.Add(panelY);
        axes.Children.Add(panelZ);

        row.Children.Add(axes);
        return row;
    }

    /// <summary>
    /// 创建轴标签按钮 + DragFloat 的组合控件。
    /// 返回 (面板 Control, DragFloat 引用) 元组，面板用于布局，DragFloat 用于监听值变化。
    /// </summary>
    private static (Control panel, DragFloat drag) CreateAxisWithButton(
        string axisLabel, SolidColorBrush color,
        float value, float speed, float resetValue)
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

        return (panel, dragInput);
    }

    // ── 四元数 ↔ 欧拉角转换（与 ImGui 版本一致，Unity 风格 ZXY 内旋顺序） ──

    /// <summary>四元数 → 欧拉角（度）。</summary>
    private static Vector3 QuaternionToEuler(Quaternion q)
    {
        q = Quaternion.Normalize(q);

        float sinrCosp = 2f * (q.W * q.X + q.Y * q.Z);
        float cosrCosp = 1f - 2f * (q.X * q.X + q.Y * q.Y);
        float roll = MathF.Atan2(sinrCosp, cosrCosp);

        float sinp = 2f * (q.W * q.Y - q.Z * q.X);
        float pitch;
        if (MathF.Abs(sinp) >= 1f)
            pitch = MathF.CopySign(MathF.PI / 2f, sinp);
        else
            pitch = MathF.Asin(sinp);

        float sinyCosp = 2f * (q.W * q.Z + q.X * q.Y);
        float cosyCosp = 1f - 2f * (q.Y * q.Y + q.Z * q.Z);
        float yaw = MathF.Atan2(sinyCosp, cosyCosp);

        return new Vector3(
            ToDegrees(roll),
            ToDegrees(pitch),
            ToDegrees(yaw));
    }

    /// <summary>欧拉角（度）→ 四元数。</summary>
    private static Quaternion EulerToQuaternion(Vector3 eulerDeg)
    {
        float roll = ToRadians(eulerDeg.X);
        float pitch = ToRadians(eulerDeg.Y);
        float yaw = ToRadians(eulerDeg.Z);

        float cr = MathF.Cos(roll * 0.5f);
        float sr = MathF.Sin(roll * 0.5f);
        float cp = MathF.Cos(pitch * 0.5f);
        float sp = MathF.Sin(pitch * 0.5f);
        float cy = MathF.Cos(yaw * 0.5f);
        float sy = MathF.Sin(yaw * 0.5f);

        return new Quaternion(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy);
    }

    private static float ToDegrees(float radians) => radians * (180f / MathF.PI);
    private static float ToRadians(float degrees) => degrees * (MathF.PI / 180f);

    // ── 实体访问 ──

    /// <summary>通过 IInspectorService 获取实体（与 SpriteRendererInspector 一致的模式）。</summary>
    private static Runtime.Scene.IEntity? GetEntityById(int entityId)
    {
        try
        {
            var context = EditorCoreModule.Context;
            if (context.TryGetService<IInspectorService>(out var inspectorService))
            {
                return inspectorService.GetEntityById(entityId);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TransformInspector] 获取实体失败: {ex.Message}");
        }
        return null;
    }
}
