using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.ImGuiFrontend.Inspectors;

/// <summary>
/// Transform 组件检查器——绘制 Position / Rotation / Scale 字段。
/// 旋转以欧拉角（度）编辑，内部存储为四元数。
/// </summary>
[InspectorOrder(0)]
public sealed class TransformInspector : ComponentTypeInspector<NNTransformData>
{
    /// <inheritdoc />
    public override int Order => 0;
    /// <inheritdoc />
    protected override bool DrawFields(ref NNTransformData data)
    {
        bool modified = false;

        // ── Position ──
        var pos = new Vector3(data.Position.X, data.Position.Y, data.Position.Z);
        if (DragFloat3("Position", ref pos))
        {
            data.Position = new NNVec3 { X = pos.X, Y = pos.Y, Z = pos.Z };
            modified = true;
        }

        // ── Rotation（以欧拉角显示/编辑）──
        var euler = QuaternionToEuler(new Quaternion(
            data.Rotation.X, data.Rotation.Y, data.Rotation.Z, data.Rotation.W));
        if (DragFloat3("Rotation", ref euler))
        {
            var quat = EulerToQuaternion(euler);
            data.Rotation = new NNQuat { X = quat.X, Y = quat.Y, Z = quat.Z, W = quat.W };
            modified = true;
        }

        // ── Scale ──
        var scl = new Vector3(data.Scale.X, data.Scale.Y, data.Scale.Z);
        if (DragFloat3("Scale", ref scl))
        {
            data.Scale = new NNVec3 { X = scl.X, Y = scl.Y, Z = scl.Z };
            modified = true;
        }

        return modified;
    }

    /// <summary>绘制三列 DragFloat（XYZ 标签），修改时返回 true。</summary>
    private static bool DragFloat3(string label, ref Vector3 value)
    {
        bool modified = false;
        ImGui.PushID(label);

        float availWidth = ImGui.GetContentRegionAvail().X;
        float itemWidth = (availWidth - 60f) / 3f;
        if (itemWidth < 40f) itemWidth = 40f;

        // 标签
        ImGui.Text(label);
        ImGui.SameLine(100f);

        // X
        ImGui.PushItemWidth(itemWidth);
        ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(1f, 0.4f, 0.4f, 1f));
        ImGui.Text("X");
        ImGui.PopStyleColor();
        ImGui.SameLine(0, 2f);
        modified |= ImGui.DragFloat("##X", ref value.X, 0.05f);
        ImGui.PopItemWidth();

        ImGui.SameLine(0, 4f);

        // Y
        ImGui.PushItemWidth(itemWidth);
        ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.4f, 1f, 0.4f, 1f));
        ImGui.Text("Y");
        ImGui.PopStyleColor();
        ImGui.SameLine(0, 2f);
        modified |= ImGui.DragFloat("##Y", ref value.Y, 0.05f);
        ImGui.PopItemWidth();

        ImGui.SameLine(0, 4f);

        // Z
        ImGui.PushItemWidth(itemWidth);
        ImGui.PushStyleColor(ImGuiCol.Text, new Vector4(0.4f, 0.4f, 1f, 1f));
        ImGui.Text("Z");
        ImGui.PopStyleColor();
        ImGui.SameLine(0, 2f);
        modified |= ImGui.DragFloat("##Z", ref value.Z, 0.05f);
        ImGui.PopItemWidth();

        ImGui.PopID();
        return modified;
    }

    /// <summary>四元数 → 欧拉角（度），Unity 风格 ZXY 内旋顺序。</summary>
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
            MathHelper.ToDegrees(roll),
            MathHelper.ToDegrees(pitch),
            MathHelper.ToDegrees(yaw));
    }

    /// <summary>欧拉角（度）→ 四元数，Unity 风格 ZXY 内旋顺序。</summary>
    private static Quaternion EulerToQuaternion(Vector3 eulerDeg)
    {
        float roll = MathHelper.ToRadians(eulerDeg.X);
        float pitch = MathHelper.ToRadians(eulerDeg.Y);
        float yaw = MathHelper.ToRadians(eulerDeg.Z);

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
}

/// <summary>角度/弧度转换辅助。</summary>
internal static class MathHelper
{
    public static float ToDegrees(float radians) => radians * (180f / MathF.PI);
    public static float ToRadians(float degrees) => degrees * (MathF.PI / 180f);
}
