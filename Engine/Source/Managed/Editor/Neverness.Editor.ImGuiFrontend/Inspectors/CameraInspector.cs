using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.ImGuiFrontend.Inspectors;

/// <summary>
/// Camera 组件检查器——绘制投影类型、裁剪面、FOV、宽高比等字段。
/// ProjectionMatrix 为系统计算，Inspector 中仅显示（只读）。
/// </summary>
[InspectorOrder(100)]
public sealed class CameraInspector : ComponentTypeInspector<CameraComponent>
{
    /// <inheritdoc />
    public override int Order => 100;
    /// <inheritdoc />
    protected override bool DrawFields(ref CameraComponent data)
    {
        bool modified = false;

        // ── Projection Type ──
        modified |= DrawProjectionType(ref data);

        // ── Clipping Planes ──
        modified |= DrawFloat("Near Plane", ref data.NearPlane, 0.01f, 0.01f, 10000f);
        modified |= DrawFloat("Far Plane", ref data.FarPlane, 1f, 1f, 100000f);

        // ── 按投影类型显示相关字段 ──
        if (!data.IsOrthographic)
        {
            modified |= DrawFloat("Field of View", ref data.FieldOfView, 1f, 1f, 179f);
            modified |= DrawFloat("Aspect Ratio", ref data.AspectRatio, 0.01f, 0.01f, 10f);
        }
        else
        {
            modified |= DrawFloat("Ortho Size", ref data.OrthographicSize, 0.1f, 0.1f, 10000f);
        }

        // ── Projection Matrix（只读显示）──
        DrawProjectionMatrix(data.ProjectionMatrix);

        return modified;
    }

    /// <summary>绘制投影类型下拉框。</summary>
    private static bool DrawProjectionType(ref CameraComponent data)
    {
        int current = data.IsOrthographic ? 1 : 0;
        string[] items = ["Perspective", "Orthographic"];

        ImGui.Text("Projection");
        ImGui.SameLine(100f);
        ImGui.PushItemWidth(ImGui.GetContentRegionAvail().X);
        bool changed = ImGui.Combo("##Projection", ref current, items, items.Length);
        ImGui.PopItemWidth();

        if (changed)
            data.IsOrthographic = current == 1;

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
    private static void DrawProjectionMatrix(Matrix4x4 matrix)
    {
        if (!ImGui.TreeNode("Projection Matrix##projmat"))
            return;

        ImGui.BeginDisabled();

        ImGui.PushItemWidth(60f);
        ImGui.InputFloat("##m00", ref matrix.M11, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m01", ref matrix.M12, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m02", ref matrix.M13, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m03", ref matrix.M14, 0, 0, "%.3f");

        ImGui.InputFloat("##m10", ref matrix.M21, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m11", ref matrix.M22, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m12", ref matrix.M23, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m13", ref matrix.M24, 0, 0, "%.3f");

        ImGui.InputFloat("##m20", ref matrix.M31, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m21", ref matrix.M32, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m22", ref matrix.M33, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m23", ref matrix.M34, 0, 0, "%.3f");

        ImGui.InputFloat("##m30", ref matrix.M41, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m31", ref matrix.M42, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m32", ref matrix.M43, 0, 0, "%.3f"); ImGui.SameLine();
        ImGui.InputFloat("##m33", ref matrix.M44, 0, 0, "%.3f");
        ImGui.PopItemWidth();

        ImGui.EndDisabled();
        ImGui.TreePop();
    }
}
