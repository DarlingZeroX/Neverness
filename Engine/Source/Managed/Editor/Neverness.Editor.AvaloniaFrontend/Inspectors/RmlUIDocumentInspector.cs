using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets;
using Neverness.Editor.AvaloniaFrontend.PropertyEditor;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// RmlUI 文档组件检查器——编辑 DocumentAsset、标志位、排序、视图目标。
///
/// 实现细节：
/// - 通过 IInspectorService.GetEntityById 获取 IEntity
/// - 通过 IEntity.Get&lt;RmlUIDocumentComponent&gt;() 读写组件数据
/// - DocumentAsset 支持从 ContentBrowser 拖拽 .html/.rml 资产设置
/// </summary>
public class RmlUIDocumentInspector : AvaloniaInspectorBase
{
    // RmlUIDocument 组件 TypeId（FNV-1a hash of "RmlUIDocument" = 0x1593AE057DEB826B）
    private const ulong RmlUIDocumentTypeId = 0x1593AE057DEB826B;

    public override string DisplayName => "RmlUI Document";

    public override bool CanInspect(ulong typeId)
    {
        return typeId == RmlUIDocumentTypeId;
    }

    public override Control CreateInspector(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        // 从 ECS 获取实体和组件当前值
        var entity = GetEntityById((int)entityHandle);
        NNGuid currentGuid = default;
        int sortOrder = 0;
        int viewTargetIndex = 2; // Both
        bool autoLoad = true;
        bool focusable = false;
        bool receivesInput = true;

        if (entity != null && entity.IsValid && entity.Has<RmlUIDocumentComponent>())
        {
            ref var doc = ref entity.Get<RmlUIDocumentComponent>();
            currentGuid = doc.DocumentAsset;
            sortOrder = doc.SortOrder;
            viewTargetIndex = (int)doc.ViewTarget;
            autoLoad = (doc.Flags & RmlUIDocumentFlags.AutoLoad) != 0;
            focusable = (doc.Flags & RmlUIDocumentFlags.Focusable) != 0;
            receivesInput = (doc.Flags & RmlUIDocumentFlags.ReceivesInput) != 0;
        }

        var content = new StackPanel { Spacing = 0 };

        // ── GUID 只读显示（先创建，拖拽回调需要引用） ──
        var guidText = $"{currentGuid.High:X16}{currentGuid.Low:X16}";
        var guidLabel = new TextBlock
        {
            Text = guidText,
            FontSize = 10,
            FontFamily = new FontFamily("Consolas"),
            Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
            Margin = new Thickness(0, 0, 0, 2),
        };

        // ── DocumentAsset（支持拖拽） ──
        string? currentAssetName = GetCurrentAssetName(currentGuid);
        var documentArea = AssetReferenceField.Create(
            currentAssetName ?? "Drop\nHTML/RML",
            assetPath => ApplyDocumentAsset(assetPath, entityHandle, guidLabel),
            width: 120, height: 60);
        content.Children.Add(PropertyRows.Create("Document", documentArea));
        content.Children.Add(PropertyRows.Create("GUID", guidLabel));

        // ── SortOrder ──
        var sortOrderDrag = new DragFloat
        {
            Value = sortOrder,
            Speed = 1f,
            Min = -1000,
            Max = 1000,
            MinWidth = 80,
        };
        sortOrderDrag.PropertyChanged += (_, e) =>
        {
            if (e.Property != DragFloat.ValueProperty) return;
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<RmlUIDocumentComponent>()) return;
            ref var c = ref ent.Get<RmlUIDocumentComponent>();
            c.SortOrder = (int)sortOrderDrag.Value;
        };
        content.Children.Add(PropertyRows.Create("Sort Order", sortOrderDrag));

        // ── ViewTarget ──
        var viewTargetCombo = new ComboBox
        {
            ItemsSource = new[] { "Scene", "Game", "Both" },
            SelectedIndex = viewTargetIndex,
            MinWidth = 100,
            Height = 22,
        };
        viewTargetCombo.SelectionChanged += (_, _) =>
        {
            if (viewTargetCombo.SelectedIndex < 0) return;
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<RmlUIDocumentComponent>()) return;
            ref var c = ref ent.Get<RmlUIDocumentComponent>();
            c.ViewTarget = (RmlUIViewTarget)viewTargetCombo.SelectedIndex;
        };
        content.Children.Add(PropertyRows.Create("View Target", viewTargetCombo));

        // ── Flags ──
        var flagsPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 12,
            Margin = new Thickness(8, 4),
        };

        flagsPanel.Children.Add(CreateEditableCheckBox("Auto Load", autoLoad, value =>
        {
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<RmlUIDocumentComponent>()) return;
            ref var c = ref ent.Get<RmlUIDocumentComponent>();
            if (value) c.Flags |= RmlUIDocumentFlags.AutoLoad;
            else c.Flags &= ~RmlUIDocumentFlags.AutoLoad;
        }));
        flagsPanel.Children.Add(CreateEditableCheckBox("Focusable", focusable, value =>
        {
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<RmlUIDocumentComponent>()) return;
            ref var c = ref ent.Get<RmlUIDocumentComponent>();
            if (value) c.Flags |= RmlUIDocumentFlags.Focusable;
            else c.Flags &= ~RmlUIDocumentFlags.Focusable;
        }));
        flagsPanel.Children.Add(CreateEditableCheckBox("Receives Input", receivesInput, value =>
        {
            var ent = GetEntityById((int)entityHandle);
            if (ent == null || !ent.IsValid || !ent.Has<RmlUIDocumentComponent>()) return;
            ref var c = ref ent.Get<RmlUIDocumentComponent>();
            if (value) c.Flags |= RmlUIDocumentFlags.ReceivesInput;
            else c.Flags &= ~RmlUIDocumentFlags.ReceivesInput;
        }));

        content.Children.Add(PropertyRows.Create("Flags", flagsPanel));

        return CreateCollapsiblePanel("RmlUI Document", content);
    }

    // ── 拖拽处理 ──

    /// <summary>
    /// 拖拽 HTML/RML 资产后，更新 ECS 中 RmlUIDocumentComponent 的 DocumentAsset 字段。
    /// 同时更新 UI 上的 GUID 标签。
    /// </summary>
    private void ApplyDocumentAsset(string assetPath, ulong entityHandle, TextBlock guidLabel)
    {
        // 1. 从路径查找资产 GUID
        var virtualPath = new NVirtualPath(assetPath);
        if (!EditorAssetDatabase.TryGetGuid(virtualPath, out var guid) || guid.IsZero)
        {
            Console.WriteLine($"[RmlUIDocument] 无法找到资产 GUID: {assetPath}");
            return;
        }

        // 2. 校验资产类型（HTML/RML 文档）
        var meta = EditorAssetDatabase.TryGetMeta(virtualPath);
        if (meta == null)
        {
            Console.WriteLine($"[RmlUIDocument] 无法获取资产 Meta: {assetPath}");
            return;
        }

        // 允许的资产类型：HTML 文档 或 RML 文档
        var ext = System.IO.Path.GetExtension(assetPath).ToLowerInvariant();
        if (ext != ".html" && ext != ".htm" && ext != ".rml")
        {
            Console.WriteLine($"[RmlUIDocument] 资产不是 HTML/RML 类型: {assetPath} (ext={ext})");
            return;
        }

        // 3. 获取 ECS 实体和组件
        var entity = GetEntityById((int)entityHandle);
        if (entity == null || !entity.IsValid) return;
        if (!entity.Has<RmlUIDocumentComponent>()) return;

        // 4. 更新 DocumentAsset GUID（ref 返回直接写入 ECS）
        ref var doc = ref entity.Get<RmlUIDocumentComponent>();
        doc.DocumentAsset = new NNGuid { High = guid.High, Low = guid.Low };

        // 5. 更新 UI 显示
        guidLabel.Text = $"{guid.High:X16}{guid.Low:X16}";

        Console.WriteLine($"[RmlUIDocument] 文档资产已更新: {assetPath} → GUID=0x{guid.ToHexString()}");
    }

    /// <summary>
    /// 获取当前 DocumentAsset GUID 对应的资产名称（用于初始化显示）。
    /// </summary>
    private static string? GetCurrentAssetName(NNGuid guid)
    {
        if (guid.High == 0 && guid.Low == 0) return null;

        try
        {
            var nGuid = new GUID(guid.High, guid.Low);
            if (EditorAssetDatabase.TryGetPath(nGuid, out var path))
            {
                return System.IO.Path.GetFileNameWithoutExtension(path.FullPath);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[RmlUIDocument] 获取资产名称失败: {ex.Message}");
        }
        return null;
    }

    // ── UI 辅助 ──

    /// <summary>创建可编辑复选框，状态变化时回调。</summary>
    private static Control CreateEditableCheckBox(string label, bool isChecked, Action<bool> onChanged)
    {
        var checkBox = new CheckBox
        {
            Content = label,
            IsChecked = isChecked,
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };
        checkBox.IsCheckedChanged += (_, _) =>
        {
            onChanged(checkBox.IsChecked == true);
        };
        return checkBox;
    }

}
