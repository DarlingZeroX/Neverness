using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.ImGuiFrontend.Inspectors;

/// <summary>
/// Camera 组件检查器——绘制投影类型、裁剪面、FOV、宽高比等字段。
/// ProjectionMatrix 为 Native System 计算，Inspector 中仅显示（只读）。
/// </summary>
[InspectorOrder(100)]
public sealed class CameraInspector : ComponentTypeInspector<NNCameraComponentData>
{
    /// <inheritdoc />
    public override int Order => 100;
    /// <inheritdoc />
    protected override bool DrawFields(ref NNCameraComponentData data)
    {
        bool modified = false;

        // ── Projection Type ──
        modified |= DrawProjectionType(ref data);

        // ── Clipping Planes ──
        modified |= DrawFloat("Near Plane", ref data.NearPlane, 0.01f, 0.01f, 10000f);
        modified |= DrawFloat("Far Plane", ref data.FarPlane, 1f, 1f, 100000f);

        // ── 按投影类型显示相关字段 ──
        if (data.Projection == NNProjectionType.Perspective)
        {
            modified |= DrawFloat("Field of View", ref data.FovY, 1f, 1f, 179f);
            modified |= DrawFloat("Aspect Ratio", ref data.AspectRatio, 0.01f, 0.01f, 10f);
        }
        else
        {
            modified |= DrawFloat("Ortho Width", ref data.OrthoWidth, 0.1f, 0.1f, 10000f);
            modified |= DrawFloat("Ortho Height", ref data.OrthoHeight, 0.1f, 0.1f, 10000f);
        }

        // ── Projection Matrix（只读显示）──
        DrawProjectionMatrix(data.ProjectionMatrix);

        return modified;
    }

    /// <summary>绘制投影类型下拉框。</summary>
    private static bool DrawProjectionType(ref NNCameraComponentData data)
    {
        int current = (int)data.Projection;
        string[] items = ["Perspective", "Orthographic"];

        ImGui.Text("Projection");
        ImGui.SameLine(100f);
        ImGui.PushItemWidth(ImGui.GetContentRegionAvail().X);
        bool changed = ImGui.Combo("##Projection", ref current, items, items.Length);
        ImGui.PopItemWidth();

        if (changed)
            data.Projection = (NNProjectionType)current;

        return changed;
    }

    /// <summary>绘制单个浮点字段（可选范围限制）。</summary>
    private static bool DrawFloat(string label, ref float value, float speed, float min, float max)
    {
        ImGui.Text(label);
        ImGui.SameLine(100f);
        ImGui.PushItemWidth(ImGui.GetContentRegionAvail().X);
        bool changed = ImGui.DragFloat($"##{label}", ref value, speed, min, max);
        ImGui.PopItemWidth();
        return changed;
    }

    /// <summary>只读显示 4x4 投影矩阵。</summary>
    private static void DrawProjectionMatrix(NNMat4 matrix)
    {
        if (!ImGui.TreeNode("Projection Matrix##projmat"))
            return;

        ImGui.BeginDisabled();
        float m00 = matrix.M00, m01 = matrix.M01, m02 = matrix.M02, m03 = matrix.M03;
        float m10 = matrix.M10, m11 = matrix.M11, m12 = matrix.M12, m13 = matrix.M13;
        float m20 = matrix.M20, m21 = matrix.M21, m22 = matrix.M22, m23 = matrix.M23;
        float m30 = matrix.M30, m31 = matrix.M31, m32 = matrix.M32, m33 = matrix.M33;

        ImGui.PushItemWidth(60f);
        ImGui.InputFloat("##m00", ref m00, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m01", ref m01, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m02", ref m02, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m03", ref m03, 0, 0, "%.3f");

        ImGui.InputFloat("##m10", ref m10, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m11", ref m11, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m12", ref m12, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m13", ref m13, 0, 0, "%.3f");

        ImGui.InputFloat("##m20", ref m20, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m21", ref m21, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m22", ref m22, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m23", ref m23, 0, 0, "%.3f");

        ImGui.InputFloat("##m30", ref m30, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m31", ref m31, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m32", ref m32, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m33", ref m33, 0, 0, "%.3f");
        ImGui.PopItemWidth();

        ImGui.EndDisabled();
        ImGui.TreePop();
    }
}
