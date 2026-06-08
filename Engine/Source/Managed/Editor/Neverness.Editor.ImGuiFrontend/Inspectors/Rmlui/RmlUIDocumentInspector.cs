using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Assets;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Rmlui;

/// <summary>
/// RmlUI 文档组件 Inspector——编辑文档资产引用、标志位、排序。
/// 显示资产名称（如 "MainMenu.html"），不显示原始 GUID。
/// 排序在 VideoPlayer(65) 和 Camera(100) 之间。
/// </summary>
[InspectorOrder(70)]
public sealed class RmlUIDocumentInspector
    : ComponentTypeInspector<NNRmlUIDocumentComponentData>
{
    public override int Order => 70;

    protected override bool DrawFields(ref NNRmlUIDocumentComponentData data)
    {
        bool modified = false;

        // ── Document 资产引用（显示资产名，不显示 GUID）──
        modified |= DrawDocumentField(ref data);

        // ── SortOrder ──
        ImGui.Text("Sort Order");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);
        modified |= ImGui.DragInt("##SortOrder", ref data.SortOrder, 1f, -1000, 1000);
        ImGui.PopItemWidth();

        // ── ViewTarget ──
        modified |= DrawViewTarget(ref data);

        // ── Flags ──
        modified |= DrawFlags(ref data);

        return modified;
    }

    /// <summary>绘制文档资产引用（显示资产名 + 拖放接收 + 右键清除）。</summary>
    private static bool DrawDocumentField(ref NNRmlUIDocumentComponentData data)
    {
        bool modified = false;

        ImGui.Text("Document");
        ImGui.SameLine(120f);

        // 尝试从 EditorAssetDatabase 获取资产名称
        var guid = GUID.FromNative(data.DocumentAsset);
        string displayName = "None";
        if (!guid.IsZero)
        {
            // 优先显示文件名，而不是 GUID
            if (EditorAssetDatabase.TryGetPath(guid, out var virtualPath))
            {
                displayName = System.IO.Path.GetFileNameWithoutExtension(virtualPath.FullPath);
            }
            else
            {
                displayName = guid.ToHexString();
            }
        }

        // 按钮显示资产名
        ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.2f, 0.2f, 0.2f, 1f));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(0.3f, 0.3f, 0.3f, 1f));
        ImGui.Button($"{displayName}##DocumentBtn", new Vector2(200f, 0));
        ImGui.PopStyleColor(2);

        // 右键清除
        if (ImGui.BeginPopupContextItem("##DocumentCtx"))
        {
            if (!guid.IsZero && ImGui.MenuItem("Clear"))
            {
                data.DocumentAsset = default;
                modified = true;
            }
            ImGui.EndPopup();
        }

        // 拖放接收（接受 .html 资产）
        using (var target = AssetDragDrop.BeginDragDropTarget())
        {
            if (target.IsActive)
            {
                // 接受任意资产类型，后续可过滤 .html
                if (AssetDragDrop.TryAcceptDragDrop(0, out var droppedGuid, out _))
                {
                    data.DocumentAsset = droppedGuid.ToNative();
                    modified = true;
                }
            }
        }

        return modified;
    }

    /// <summary>绘制标志位复选框。</summary>
    private static bool DrawFlags(ref NNRmlUIDocumentComponentData data)
    {
        bool modified = false;
        var flags = data.Flags;

        bool autoLoad = flags.HasFlag(NNRmlUIDocumentFlags.AutoLoad);
        if (ImGui.Checkbox("Auto Load", ref autoLoad))
        {
            flags = autoLoad ? flags | NNRmlUIDocumentFlags.AutoLoad
                             : flags & ~NNRmlUIDocumentFlags.AutoLoad;
            modified = true;
        }

        bool focusable = flags.HasFlag(NNRmlUIDocumentFlags.Focusable);
        if (ImGui.Checkbox("Focusable", ref focusable))
        {
            flags = focusable ? flags | NNRmlUIDocumentFlags.Focusable
                              : flags & ~NNRmlUIDocumentFlags.Focusable;
            modified = true;
        }

        ImGui.SameLine();
        bool receivesInput = flags.HasFlag(NNRmlUIDocumentFlags.ReceivesInput);
        if (ImGui.Checkbox("Receives Input", ref receivesInput))
        {
            flags = receivesInput ? flags | NNRmlUIDocumentFlags.ReceivesInput
                                  : flags & ~NNRmlUIDocumentFlags.ReceivesInput;
            modified = true;
        }

        if (modified)
            data.Flags = flags;
        return modified;
    }

    /// <summary>绘制视图目标下拉框。</summary>
    private static bool DrawViewTarget(ref NNRmlUIDocumentComponentData data)
    {
        ImGui.Text("View Target");
        ImGui.SameLine(120f);
        ImGui.PushItemWidth(200f);

        int current = (int)data.ViewTarget;
        string[] items = ["Scene", "Game", "Both"];
        bool modified = ImGui.Combo("##ViewTarget", ref current, items, items.Length);
        if (modified)
            data.ViewTarget = (NNRmlUIViewTarget)current;

        ImGui.PopItemWidth();
        return modified;
    }
}
