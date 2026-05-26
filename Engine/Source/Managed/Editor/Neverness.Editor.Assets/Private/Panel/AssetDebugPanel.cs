using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private.Panel;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;
using System.Numerics;

namespace Neverness.Editor.Assets.Private.Panel;

/// <summary>
/// 资产数据库调试面板——可视化 EditorAssetDatabase 中所有注册资产。
/// 开发阶段用于排查资产注册、GUID/路径映射、导入状态等。
/// </summary>
public sealed class AssetDebugPanel : EditorPanel, IEditorPanel
{
    private bool _isOpen = true;
    private string _searchFilter = "";
    private int _selectedRow = -1;

    public AssetDebugPanel() : base("asset_debug", "Asset Debug")
    {
    }

    // ── IEditorPanel ──

    public string GetWindowFullName() => FontAwesome5Pro.Database + " " + GetWindowName();
    public string GetWindowName() => Title;
    public void OpenWindow(bool open) => _isOpen = open;
    public bool IsWindowOpened() => _isOpen;
    public bool IsAsync() => false;
    public void OnUpdate(float delta) { }
    public void OnFixedUpdate() { }

    // ── 渲染 ──

    public void OnGUI()
    {
        if (!_isOpen) return;

        if (!ImGui.Begin(GetWindowFullName(), ref _isOpen, ImGuiWindowFlags.MenuBar))
        {
            ImGui.End();
            return;
        }

        DrawMenuBar();
        DrawAssetTable();

        ImGui.End();
    }

    /// <summary>顶部菜单栏：统计信息 + 搜索过滤。</summary>
    private void DrawMenuBar()
    {
        if (!ImGui.BeginMenuBar()) return;

        ImGui.Text($"Assets: {EditorAssetDatabase.AssetCount}");

        ImGui.Separator();

        // 搜索过滤框
        ImGui.SetNextItemWidth(200);
        ImGui.InputText("##search", ref _searchFilter, 256);

        ImGui.EndMenuBar();
    }

    /// <summary>主表格：列出所有注册资产。</summary>
    private void DrawAssetTable()
    {
        var allAssets = EditorAssetDatabase.AllAssets;
        var filter = _searchFilter.AsSpan();

        var flags = ImGuiTableFlags.Resizable
                  | ImGuiTableFlags.Sortable
                  | ImGuiTableFlags.ScrollY
                  | ImGuiTableFlags.RowBg
                  | ImGuiTableFlags.BordersOuter
                  | ImGuiTableFlags.BordersV;

        if (!ImGui.BeginTable("##asset_table", 7, flags))
            return;

        // 列定义
        ImGui.TableSetupColumn("#", ImGuiTableColumnFlags.WidthFixed, 40);
        ImGui.TableSetupColumn("GUID", ImGuiTableColumnFlags.WidthFixed, 260);
        ImGui.TableSetupColumn("Path", ImGuiTableColumnFlags.WidthStretch);
        ImGui.TableSetupColumn("Type", ImGuiTableColumnFlags.WidthFixed, 100);
        ImGui.TableSetupColumn("Importer", ImGuiTableColumnFlags.WidthFixed, 130);
        ImGui.TableSetupColumn("Dirty", ImGuiTableColumnFlags.WidthFixed, 50);
        ImGui.TableSetupColumn("Labels", ImGuiTableColumnFlags.WidthStretch);
        ImGui.TableSetupScrollFreeze(0, 1);
        ImGui.TableHeadersRow();

        for (int i = 0; i < allAssets.Count; i++)
        {
            var guid = allAssets[i];

            // 取基础信息
            EditorAssetDatabase.TryGetPath(guid, out var virtualPath);
            var pathStr = virtualPath.FullPath;

            // 搜索过滤
            if (filter.Length > 0
                && !pathStr.AsSpan().Contains(filter, StringComparison.OrdinalIgnoreCase)
                && !guid.ToUuidString().AsSpan().Contains(filter, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            var meta = EditorAssetDatabase.TryGetMeta(guid);
            var typeId = EditorAssetDatabase.GetTypeId(guid);
            var isDirty = EditorAssetDatabase.IsDirty(guid);

            ImGui.PushID(i);
            ImGui.TableNextRow();

            // 选中行高亮
            if (_selectedRow == i)
            {
                ImGui.TableSetBgColor(ImGuiTableBgTarget.RowBg1, ImGui.GetColorU32(new Vector4(0.2f, 0.4f, 0.7f, 0.3f)));
            }

            // #
            ImGui.TableSetColumnIndex(0);
            ImGui.TextUnformatted((i + 1).ToString());

            // GUID
            ImGui.TableSetColumnIndex(1);
            ImGui.TextUnformatted(guid.ToUuidString());

            // Path
            ImGui.TableSetColumnIndex(2);
            ImGui.TextUnformatted(pathStr);

            // Type
            ImGui.TableSetColumnIndex(3);
            ImGui.TextUnformatted(TypeIdToName(typeId));

            // Importer
            ImGui.TableSetColumnIndex(4);
            ImGui.TextUnformatted(meta?.Importer ?? "-");

            // Dirty
            ImGui.TableSetColumnIndex(5);
            if (isDirty)
            {
                ImGui.TextColored(new Vector4(1f, 0.6f, 0.2f, 1f), "DIRTY");
            }

            // Labels
            ImGui.TableSetColumnIndex(6);
            var labels = EditorAssetDatabase.GetLabels(guid);
            if (labels.Count > 0)
            {
                ImGui.TextUnformatted(string.Join(", ", labels));
            }

            // 点击选中
            if (ImGui.IsItemClicked())
            {
                _selectedRow = i;
            }

            ImGui.PopID();
        }

        ImGui.EndTable();
    }

    /// <summary>将 TypeId 数值转为可读名称。</summary>
    private static string TypeIdToName(ulong typeId) => typeId switch
    {
        AssetTypeId.Texture2D => "Texture2D",
        AssetTypeId.Mesh => "Mesh",
        AssetTypeId.AudioClip => "AudioClip",
        AssetTypeId.Material => "Material",
        AssetTypeId.Shader => "Shader",
        AssetTypeId.Scene => "Scene",
        AssetTypeId.Prefab => "Prefab",
        AssetTypeId.Animation => "Animation",
        AssetTypeId.LuaScript => "LuaScript",
        _ => $"Unknown({typeId})"
    };
}
