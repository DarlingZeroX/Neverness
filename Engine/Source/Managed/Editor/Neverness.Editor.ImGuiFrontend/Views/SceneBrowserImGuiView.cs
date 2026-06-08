using Hexa.NET.ImGui;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Editor.Scene.Public;

namespace Neverness.Editor.ImGuiFrontend.Views;

/// <summary>
/// 场景浏览器 ImGui View——读取 SceneBrowserViewModel，渲染 ImGui 树形 UI。
///
/// Phase 3 验证对象：复杂 MVVM 面板，验证树形结构、选中状态、拖拽、右键菜单。
/// </summary>
public class SceneBrowserImGuiView : PanelViewBase
{
    private SceneBrowserViewModel? _viewModel;
    private SceneBrowserController? _controller;

    private float _itemHeight = 22f;

    /// <summary>延迟删除的实体句柄（0 = 无待删除）。</summary>
    private ulong _pendingDeleteHandle;

    public SceneBrowserImGuiView()
        : base("Scene Browser", FontAwesome5Pro.Window + " Scene Browser")
    {
    }

    /// <summary>设置 Controller（由 CompositionRoot 调用）。</summary>
    public void SetController(SceneBrowserController controller)
    {
        _controller = controller;
    }

    public override Type ViewModelType => typeof(SceneBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (SceneBrowserViewModel)viewModel;
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
    }

    /// <summary>每帧更新——自动刷新实体树（版本轮询，无变化时几乎零开销）。</summary>
    public override void OnUpdate(float delta)
    {
        // 处理延迟删除（避免在渲染过程中修改实体树）
        if (_pendingDeleteHandle != 0)
        {
            _controller?.DeleteEntity(_pendingDeleteHandle);
            _pendingDeleteHandle = 0;
        }

        // 自动刷新实体树（TryRefreshHierarchy 内部有版本轮询，无变化时返回 false）
        _controller?.RefreshTree();
    }

    public override void Render()
    {
        if (_viewModel == null) return;

        if (!ImGui.Begin(GetWindowFullName()))
        {
            ImGui.End();
            return;
        }

        if (!_viewModel.HasScene)
        {
            ImGui.TextDisabled("No active scene.");
            ImGui.End();
            return;
        }

        RenderToolbar();
        RenderSearchBar();
        RenderEntityTree();

        ImGui.End();
    }

    // ── 工具栏 ──
    private void RenderToolbar()
    {
        if (ImGui.Button("Refresh"))
            _controller?.RefreshTree();
        ImGui.SameLine();
        if (ImGui.Button("+"))
            _controller?.ExpandAll();
        ImGui.SameLine();
        if (ImGui.Button("-"))
            _controller?.CollapseAll();
        ImGui.SameLine();
        ImGui.TextDisabled($"{_viewModel!.NodeCount}");
    }

    // ── 搜索栏 ──
    private void RenderSearchBar()
    {
        var searchText = _viewModel!.SearchText;
        ImGui.SetNextItemWidth(ImGui.GetContentRegionAvail().X);
        if (ImGui.InputTextWithHint("##sceneSearch", "Search entities...", ref searchText, 256))
        {
            _controller?.SetSearchText(searchText);
        }
        ImGui.Separator();
    }

    // ── 实体树（手动虚拟化）──
    private void RenderEntityTree()
    {
        var visible = _viewModel!.VisibleNodes;

        ImGui.BeginChild("##hierarchy", System.Numerics.Vector2.Zero,
            ImGuiChildFlags.None, ImGuiWindowFlags.HorizontalScrollbar);

        // 背景右键菜单（在子窗口中调用）
        RenderBackgroundContextMenu();

        if (visible.Count == 0)
        {
            ImGui.TextDisabled("No entities.");
            ImGui.EndChild();
            return;
        }

        float totalHeight = visible.Count * _itemHeight;

        // 占位空间控制滚动条范围
        ImGui.Dummy(new System.Numerics.Vector2(1, totalHeight));

        // 计算可见区域的 first/last 行索引
        float scrollY = ImGui.GetScrollY();
        float viewHeight = ImGui.GetWindowHeight();
        int firstVisible = Math.Max(0, (int)(scrollY / _itemHeight) - 1);
        int lastVisible = Math.Min(visible.Count - 1,
            firstVisible + (int)(viewHeight / _itemHeight) + 2);

        // 移动游标到第一个可见行
        ImGui.SetCursorPosY(firstVisible * _itemHeight);

        for (int i = firstVisible; i <= lastVisible; i++)
        {
            var node = visible[i];
            if (node == null) continue;
            RenderNode(node);
        }

        ImGui.EndChild();
    }

    // ── 单个节点渲染 ──
    private void RenderNode(EntityNodeVM node)
    {
        bool hasChildren = node.HasChildren;
        bool expanded = _viewModel!.IsExpanded(node.Handle);
        bool selected = _viewModel.SelectedEntityHandle == node.Handle;

        // 缩进
        float indent = node.Depth * 16f;
        ImGui.SetCursorPosX(ImGui.GetCursorPosX() + indent);

        var flags = ImGuiTreeNodeFlags.SpanAvailWidth
                  | ImGuiTreeNodeFlags.FramePadding
                  | ImGuiTreeNodeFlags.OpenOnArrow
                  | ImGuiTreeNodeFlags.OpenOnDoubleClick;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags.Leaf | ImGuiTreeNodeFlags.NoTreePushOnOpen;
        if (expanded)
            flags |= ImGuiTreeNodeFlags.DefaultOpen;
        if (selected)
            flags |= ImGuiTreeNodeFlags.Selected;

        ImGui.SetNextItemOpen(expanded, ImGuiCond.Always);

        string label = $"{node.Handle}##{node.Handle}";
        bool nowOpen = ImGui.TreeNodeEx(label, flags);

        // 展开状态变化
        if (hasChildren && nowOpen != expanded)
            _controller?.ToggleExpand(node.Handle);

        // 点击选中
        if (ImGui.IsItemClicked(ImGuiMouseButton.Left) && !ImGui.IsItemToggledOpen())
        {
            _controller?.SelectEntity(node.Handle, ImGui.GetIO().KeyCtrl);
        }

        // 拖拽源
        if (ImGui.BeginDragDropSource())
        {
            unsafe
            {
                ulong entityHandle = node.Handle;
                ImGui.SetDragDropPayload("ENTITY", (void*)&entityHandle, (nuint)sizeof(ulong));
            }
            ImGui.Text(node.Name);
            ImGui.EndDragDropSource();
        }

        // 拖拽目标
        if (hasChildren && ImGui.BeginDragDropTarget())
        {
            var payload = ImGui.AcceptDragDropPayload("ENTITY");
            if (!payload.IsNull)
            {
                unsafe
                {
                    ulong dragged = *(ulong*)payload.Data;
                    if (dragged != node.Handle)
                    {
                        _controller?.ReparentEntity(dragged, node.Handle);
                    }
                }
            }
            ImGui.EndDragDropTarget();
        }

        // 右键菜单
        if (ImGui.BeginPopupContextItem($"##ctx_{node.Handle}"))
        {
            ImGui.Text(node.Name);
            ImGui.Separator();
            if (ImGui.MenuItem("Rename"))
                _controller?.BeginRename(node.Handle);
            if (ImGui.MenuItem("Duplicate"))
                _controller?.DuplicateEntity(node.Handle);
            if (ImGui.MenuItem("Delete"))
            {
                // 延迟删除：先关闭弹出菜单，下一帧再删除实体
                // 避免在渲染过程中修改实体树导致 ImGui 状态不一致
                ImGui.CloseCurrentPopup();
                _pendingDeleteHandle = node.Handle;
            }
            ImGui.EndPopup();
        }

        if (nowOpen && hasChildren)
            ImGui.TreePop();
    }

    // ── 背景右键菜单 ──
    private void RenderBackgroundContextMenu()
    {
        var ctx = ContextMenuManager.Instance;

        // 注入运行时上下文（供 Add Entity 命令读取）
        var world = SceneModule.GetActiveWorld();
        if (world != null)
        {
            ctx.SetContext(SceneBrowserContextMenu.KeyActiveWorld, world);
        }

        // 使用 ContextMenuManager 的渲染方法（与 ContentBrowser 一致）
        ctx.RenderWindowContextMenu(SceneBrowserContextMenu.BackgroundId);
    }

    private void OnPropertyChanged(string propertyName)
    {
        // ImGui 即时模式，每帧重新读取 ViewModel
    }
}
