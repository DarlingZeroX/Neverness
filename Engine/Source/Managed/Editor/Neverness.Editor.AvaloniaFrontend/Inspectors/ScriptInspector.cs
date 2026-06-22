using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Script.Private;
using Neverness.Gameplay;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Script 组件检查器——编辑 ScriptComponent。
///
/// 3 种 UI 状态（与 ImGui 版 EcsScriptInspector 一致）：
/// - None:      ScriptTypeId == 0 → "None (drop .cs script here)"
/// - Uncompiled: ScriptTypeId != 0 且 ScriptRegistry 未注册 → "Uncompiled (ClassName)"
/// - Bound:     ScriptTypeId != 0 且 ScriptRegistry 已注册 → "ClassName"
///
/// 支持：
/// - ContentBrowser .cs 脚本资产拖拽绑定
/// - 下拉菜单选择已注册的脚本类型
/// - 右键菜单清除脚本绑定
/// </summary>
public class ScriptInspector : AvaloniaInspectorBase
{
    /// <summary>Script 组件 TypeId（FNV-1a hash of "Script" = 0x9565553D163FC92A）。</summary>
    private const ulong ScriptTypeId = 0x9565553D163FC92A;

    public override string DisplayName => "Script";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == ScriptTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        // 从 ECS 获取实体和组件当前值
        var entity = GetEntityById((int)entityHandle);
        ulong currentScriptTypeId = 0;

        if (entity != null && entity.IsValid && entity.Has<ScriptComponent>())
        {
            ref var script = ref entity.Get<ScriptComponent>();
            currentScriptTypeId = script.ScriptTypeId;
        }

        var content = new StackPanel { Spacing = 0 };

        // ── 脚本名称显示（3 种状态） ──
        var (scriptName, isBound) = ResolveScriptName(currentScriptTypeId);
        var statusLabel = new TextBlock
        {
            Text = scriptName,
            FontSize = 12,
            Foreground = isBound ? ColorText : new SolidColorBrush(Color.Parse("#FFFFCC66")),
            Margin = new Thickness(8, 4),
        };
        content.Children.Add(CreatePropertyRow("Script", statusLabel));

        // ── ScriptTypeId 只读显示 ──
        var typeIdLabel = new TextBlock
        {
            Text = currentScriptTypeId == 0 ? "—" : $"0x{currentScriptTypeId:X16}",
            FontSize = 10,
            FontFamily = new FontFamily("Consolas"),
            Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
            Margin = new Thickness(8, 2),
        };
        content.Children.Add(CreatePropertyRow("TypeId", typeIdLabel));

        // ── 脚本选择下拉（已注册的脚本类型列表） ──
        var scriptCombo = CreateScriptTypeComboBox(currentScriptTypeId, entityHandle,
            statusLabel, typeIdLabel);
        content.Children.Add(CreatePropertyRow("Type", scriptCombo));

        // ── 拖拽目标区域 ──
        var dropTarget = CreateScriptDropTarget(currentScriptTypeId, entityHandle,
            statusLabel, typeIdLabel);
        content.Children.Add(dropTarget);

        return CreateCollapsiblePanel("Script", content);
    }

    // ── 脚本类型解析 ──

    /// <summary>从 ScriptTypeId 解析脚本名称（3 种状态）。</summary>
    private static (string Name, bool IsBound) ResolveScriptName(ulong scriptTypeId)
    {
        if (scriptTypeId == 0)
            return ("None", false);

        var registry = GameplayContext.Current?.ScriptRegistry;
        var scriptInfo = registry?.FindByTypeId(scriptTypeId);

        if (scriptInfo != null)
            return (scriptInfo.Name, true);

        return ("Uncompiled", false);
    }

    // ── 脚本类型下拉 ──

    /// <summary>创建脚本类型选择下拉框。</summary>
    private static Control CreateScriptTypeComboBox(ulong currentTypeId, ulong entityHandle,
        TextBlock statusLabel, TextBlock typeIdLabel)
    {
        var registry = GameplayContext.Current?.ScriptRegistry;
        var allScripts = registry?.GetAllScripts().ToList() ?? [];

        // 构建下拉选项：[None] + 所有已注册脚本
        var items = new List<string> { "(None)" };
        items.AddRange(allScripts.Select(s => s.Name));

        // 计算当前选中索引
        int selectedIndex = 0;
        if (currentTypeId != 0)
        {
            var idx = allScripts.FindIndex(s => s.TypeId == currentTypeId);
            selectedIndex = idx >= 0 ? idx + 1 : 0;
        }

        var comboBox = new ComboBox
        {
            ItemsSource = items,
            SelectedIndex = selectedIndex,
            MinWidth = 150,
            Height = 22,
            HorizontalAlignment = HorizontalAlignment.Stretch,
        };

        comboBox.SelectionChanged += (_, _) =>
        {
            if (comboBox.SelectedIndex < 0) return;

            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<ScriptComponent>()) return;

            ref var sc = ref ent.Get<ScriptComponent>();

            if (comboBox.SelectedIndex == 0)
            {
                // 选择 (None) → 清除脚本
                sc.ScriptTypeId = 0;
            }
            else
            {
                // 选择已注册脚本
                var selected = allScripts[comboBox.SelectedIndex - 1];
                sc.ScriptTypeId = selected.TypeId;
            }

            // 刷新 UI
            var (name, bound) = ResolveScriptName(sc.ScriptTypeId);
            statusLabel.Text = name;
            statusLabel.Foreground = bound ? ColorText : new SolidColorBrush(Color.Parse("#FFFFCC66"));
            typeIdLabel.Text = sc.ScriptTypeId == 0 ? "—" : $"0x{sc.ScriptTypeId:X16}";
        };

        return comboBox;
    }

    // ── 拖拽目标 ──

    /// <summary>创建脚本拖拽目标区域。</summary>
    private static Control CreateScriptDropTarget(ulong currentTypeId, ulong entityHandle,
        TextBlock statusLabel, TextBlock typeIdLabel)
    {
        var placeholderText = currentTypeId == 0
            ? "Drop .cs script here"
            : "Drop .cs to change";

        var dropTarget = CreateAssetDropTarget(
            placeholderText,
            assetPath => ApplyScriptAsset(assetPath, entityHandle, statusLabel, typeIdLabel),
            width: 160, height: 48);

        // 包装在带边距的容器中
        var wrapper = new DockPanel
        {
            Margin = new Thickness(8, 6, 8, 4),
        };
        wrapper.Children.Add(dropTarget);
        return wrapper;
    }

    /// <summary>拖拽 .cs 脚本资产后，更新 ECS 中 ScriptComponent 的 ScriptTypeId。</summary>
    private static void ApplyScriptAsset(string assetPath, ulong entityHandle,
        TextBlock statusLabel, TextBlock typeIdLabel)
    {
        // 1. 从路径查找资产 GUID
        var virtualPath = new NVirtualPath(assetPath);
        if (!EditorAssetDatabase.TryGetGuid(virtualPath, out var guid) || guid.IsZero)
        {
            Console.WriteLine($"[ScriptInspector] 无法找到资产 GUID: {assetPath}");
            return;
        }

        // 2. 校验资产类型必须是 C# 脚本
        var meta = EditorAssetDatabase.TryGetMeta(virtualPath);
        if (meta == null || meta.AssetTypeId != AssetTypeId.CSharpScript)
        {
            Console.WriteLine($"[ScriptInspector] 资产不是 C# 脚本: {assetPath} (TypeId={meta?.AssetTypeId})");
            return;
        }

        // 3. 通过 ScriptAssetIndex 查询 ScriptTypeId
        ulong scriptTypeId = 0;
        if (ScriptAssetIndex.Instance.TryGetScriptTypeId(guid, out var foundTypeId))
        {
            scriptTypeId = foundTypeId;
        }
        else if (ScriptAssetIndex.Instance.TryGetFullName(guid, out var fullName))
        {
            // ScriptAssetIndex 未直接映射 TypeId，通过 ScriptRegistry 查找
            var registry = GameplayContext.Current?.ScriptRegistry;
            var typeInfo = registry?.FindByName(fullName);
            if (typeInfo != null)
            {
                scriptTypeId = typeInfo.TypeId;
            }
            else
            {
                Console.WriteLine($"[ScriptInspector] 脚本未编译或未注册: {fullName}");
                // 仍然设置 FullName 的 hash，标记为 Uncompiled 状态
                scriptTypeId = Fnv1a64(fullName);
            }
        }

        if (scriptTypeId == 0)
        {
            Console.WriteLine($"[ScriptInspector] 无法解析脚本 TypeId: {assetPath}");
            return;
        }

        // 4. 更新 ECS 组件
        var entity = GetEntityById((int)entityHandle);
        if (entity == null || !entity.IsValid || !entity.Has<ScriptComponent>()) return;

        ref var sc = ref entity.Get<ScriptComponent>();
        sc.ScriptTypeId = scriptTypeId;

        // 5. 刷新 UI
        var (name, bound) = ResolveScriptName(scriptTypeId);
        statusLabel.Text = name;
        statusLabel.Foreground = bound ? ColorText : new SolidColorBrush(Color.Parse("#FFFFCC66"));
        typeIdLabel.Text = $"0x{scriptTypeId:X16}";

        Console.WriteLine($"[ScriptInspector] 脚本已绑定: {assetPath} → TypeId=0x{scriptTypeId:X16} ({name})");
    }

    // ── 工具方法 ──

    /// <summary>FNV-1a 64-bit hash（与 ScriptRegistry.CalculateTypeId 一致）。</summary>
    private static ulong Fnv1a64(string name)
    {
        ulong hash = 14695981039346656037UL;
        foreach (var c in name)
        {
            hash ^= (byte)c;
            hash *= 1099511628211UL;
        }
        return hash;
    }

    /// <summary>通过 IInspectorService 获取实体。</summary>
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
            Console.WriteLine($"[ScriptInspector] 获取实体失败: {ex.Message}");
        }
        return null;
    }
}
