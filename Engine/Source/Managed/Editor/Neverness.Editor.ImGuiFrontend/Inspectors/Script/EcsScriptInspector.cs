// ============================================================================
// EcsScriptInspector.cs - ECS 脚本组件 Inspector
// ============================================================================
// 在 DetailInspector 中显示和编辑 NNScriptComponentData。
// 支持 ContentBrowser .cs 脚本资产拖拽绑定。
//
// UI 状态机：
// - None: ScriptTypeId == 0 → "None (drop .cs script here)"
// - Uncompiled: ScriptTypeId != 0 且 ScriptRegistry 未注册 → "Uncompiled (ClassName)"
// - Bound: ScriptTypeId != 0 且 ScriptRegistry 已注册 → "ClassName" + Enabled
// ============================================================================

using Hexa.NET.ImGui;
using System.Numerics;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Core.Public.Inspector;
using Neverness.Editor.Script.Private;
using Neverness.Gameplay;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Vector2 = System.Numerics.Vector2;
using Vector4 = System.Numerics.Vector4;

namespace Neverness.Editor.ImGuiFrontend.Inspectors.Script;

/// <summary>
/// ECS 脚本组件 Inspector——编辑 NNScriptComponentData。
/// 支持拖拽 .cs 脚本资产绑定脚本类型。
/// </summary>
public sealed class EcsScriptInspector : ComponentTypeInspector<NNScriptComponentData>
{
    /// <inheritdoc />
    public override int Order => 200;

    /// <inheritdoc />
    protected override bool DrawFields(ref NNScriptComponentData data)
    {
        bool modified = false;

        // ====================================================================
        // 1. 脚本名称显示（3 种状态）
        // ====================================================================
        string scriptName;
        bool isBound = false;

        if (data.ScriptTypeId == 0)
        {
            // 状态 1: None
            scriptName = "None";
        }
        else
        {
            // 尝试从 ScriptRegistry 反查
            var registry = GameplayContext.Current?.ScriptRegistry;
            var scriptInfo = registry?.FindByTypeId(data.ScriptTypeId);

            if (scriptInfo != null)
            {
                // 状态 3: Bound
                scriptName = scriptInfo.Name;
                isBound = true;
            }
            else
            {
                // 状态 2: Uncompiled
                scriptName = "Uncompiled";
            }
        }

        ImGui.Text($"Script: {scriptName}");
        if (!isBound && data.ScriptTypeId != 0)
        {
            ImGui.SameLine();
            ImGui.TextColored(new Vector4(1f, 0.8f, 0.2f, 1f), "(uncompiled)");
        }

        // ====================================================================
        // 2. Enabled Checkbox
        // ====================================================================
        if (data.ScriptTypeId != 0)
        {
            bool enabled = data.Enabled != 0;
            if (ImGui.Checkbox("Enabled", ref enabled))
            {
                data.Enabled = enabled ? (byte)1 : (byte)0;
                modified = true;
            }
        }

        // ====================================================================
        // 3. 拖拽目标区域
        // ====================================================================
        ImGui.Separator();

        var dropText = data.ScriptTypeId == 0
            ? "Drop .cs script here"
            : "Drop .cs to change script";

        // 按钮作为拖拽目标（ImGui 的 Drop Target 绑定到上一个 Widget）
        ImGui.Button(dropText, new Vector2(-1, 0));

        using (var target = AssetDragDrop.BeginDragDropTarget())
        {
            if (target.IsActive)
            {
                if (AssetDragDrop.TryAcceptDragDrop(AssetDragDrop.Script, out var guid, out _))
                {
                    // 通过 ScriptAssetIndex 查询 ScriptTypeId
                    if (ScriptAssetIndex.Instance.TryGetScriptTypeId(guid, out var scriptTypeId))
                    {
                        data.ScriptTypeId = scriptTypeId;
                        data.Enabled = 1;
                        modified = true;
                    }
                    else
                    {
                        // ScriptAssetIndex 未找到，尝试 FullName fallback
                        if (ScriptAssetIndex.Instance.TryGetFullName(guid, out var fullName))
                        {
                            var registry2 = GameplayContext.Current?.ScriptRegistry;
                            var typeInfo = registry2?.FindByName(fullName);
                            if (typeInfo != null)
                            {
                                data.ScriptTypeId = typeInfo.TypeId;
                                data.Enabled = 1;
                                modified = true;
                            }
                        }
                    }
                }
            }
        }

        // ====================================================================
        // 4. 右键菜单
        // ====================================================================
        if (ImGui.BeginPopupContextItem())
        {
            if (data.ScriptTypeId != 0)
            {
                if (ImGui.MenuItem("Remove Script"))
                {
                    data.ScriptTypeId = 0;
                    data.Enabled = 1;
                    modified = true;
                }
            }
            ImGui.EndPopup();
        }

        return modified;
    }
}
