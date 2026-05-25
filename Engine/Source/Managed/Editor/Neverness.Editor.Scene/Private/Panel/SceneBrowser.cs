using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.Cache;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using System.Numerics;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// 场景层级面板——基于 Snapshot 驱动的层级浏览器。
///
/// 数据流：
///   OnUpdate → SceneHierarchyCache.TryRefresh (版本轮询 + 按需拉取)
///   OnGUI → 手动虚拟化渲染（scrollY / itemHeight 裁剪 + DFS depth 跳跃）
///
/// 支持：展开/折叠、搜索过滤、拖放重设 Parent、右键上下文菜单。
/// </summary>
public class SceneBrowser : IEditorPanel
{
    private bool _isOpen = true;
    private readonly SceneHierarchyCache _cache = new();
    private readonly SceneManager _sceneManager;
    private ulong _sceneHandle;
    private float _itemHeight = 22f;

    /// <summary>搜索文本缓冲区（ImGui InputText 专用）。</summary>
    private string _searchBuf = "";

    /// <summary>正在重命名的实体句柄（0 = 无重命名）。</summary>
    private ulong _renamingEntity;

    /// <summary>重命名输入缓冲区。</summary>
    private string _renameBuf = "";

    /// <summary>编辑器事件总线引用（用于发出 SelectionChanged 事件）。</summary>
    private IEditorEventBus? _eventBus;

    public SceneBrowser(SceneManager sceneManager)
        : this(sceneManager, 0)
    {
    }

    public SceneBrowser(SceneManager sceneManager, ulong sceneHandle)
    {
        _sceneManager = sceneManager;
        _sceneHandle = sceneHandle;
    }

    /// <summary>设置或获取当前关联的场景句柄。</summary>
    public ulong SceneHandle
    {
        get => _sceneHandle;
        set => _sceneHandle = value;
    }

    /// <summary>设置编辑器事件总线（用于发出 SelectionChanged 事件）。</summary>
    public IEditorEventBus? EventBus
    {
        get => _eventBus;
        set => _eventBus = value;
    }

    /// <summary>暴露场景层级缓存引用（供 <see cref="DetailInspector"/> 直接查询选中状态）。</summary>
    public SceneHierarchyCache Cache => _cache;

    // ── IEditorPanel ──

    public void OnUpdate(float delta)
    {
        if (_sceneHandle == 0) return;
        _cache.TryRefresh(_sceneHandle);
    }

    public void OnFixedUpdate() { }

    public bool IsAsync() => false;

    public void OnGUI()
    {
        if (!_isOpen) return;

        if (!ImGui.Begin(GetWindowFullName(), ImGuiWindowFlags.None))
        {
            ImGui.End();
            return;
        }

        if (_sceneHandle == 0)
        {
            ImGui.TextDisabled("No active scene.");
            ImGui.End();
            return;
        }

        DrawToolbar();
        DrawHierarchyTree();

        ImGui.End();
    }

    public string GetWindowFullName() => FontAwesome5Pro.Window + " " + GetWindowName();

    public string GetWindowName() => "SceneBrowser";

    public void OpenWindow(bool open) => _isOpen = open;

    public bool IsWindowOpened() => _isOpen;

    // ── 工具栏 ──

    private void DrawToolbar()
    {
        // 搜索框
        ImGui.SetNextItemWidth(ImGui.GetContentRegionAvail().X - 120);
        if (ImGui.InputTextWithHint("##sceneSearch", "Search entities...", ref _searchBuf, 256))
        {
            _cache.SetSearch(_searchBuf);
        }

        // 展开/折叠按钮
        ImGui.SameLine();
        if (ImGui.Button("+##ExpandAll"))
            _cache.ExpandAll();

        ImGui.SameLine();
        if (ImGui.Button("-##CollapseAll"))
            _cache.CollapseAll();

        // 节点计数
        ImGui.SameLine();
        ImGui.TextDisabled($"{_cache.NodeCount}");
    }

    // ── 层级树（手动虚拟化）─────────────────────────────────────────

    private void DrawHierarchyTree()
    {
        _cache.RebuildVisibleList();
        var visible = _cache.VisibleList;

        if (visible.Count == 0)
        {
            ImGui.TextDisabled("No entities.");
            DrawBackgroundContextMenu();
            return;
        }

        float totalHeight = visible.Count * _itemHeight;

        ImGui.BeginChild("##hierarchy", Vector2.Zero, ImGuiChildFlags.None,
            ImGuiWindowFlags.HorizontalScrollbar);

        // 占位空间控制滚动条范围
        ImGui.Dummy(new Vector2(1, totalHeight));

        // 计算可见区域的 first/last 行索引
        float scrollY = ImGui.GetScrollY();
        float viewHeight = ImGui.GetWindowHeight();
        int firstVisible = Math.Max(0, (int)(scrollY / _itemHeight) - 1);
        int lastVisible = Math.Min(visible.Count - 1,
            firstVisible + (int)(viewHeight / _itemHeight) + 2);

        // 移动游标到第一个可见行
        ImGui.SetCursorPosY(firstVisible * _itemHeight);

        for (int vi = firstVisible; vi <= lastVisible; vi++)
        {
            int nodeIdx = visible[vi];
            if (nodeIdx < 0 || nodeIdx >= _cache.AllNodes.Length) continue;
            DrawNode(_cache.AllNodes[nodeIdx]);
        }

        DrawBackgroundContextMenu();

        ImGui.EndChild();
    }

    /// <summary>空白区域右键菜单——添加实体等操作。</summary>
    private void DrawBackgroundContextMenu()
    {
        if (ImGui.IsWindowHovered() && !ImGui.IsAnyItemHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right))
            ImGui.OpenPopup("##scene_bg_ctx");

        if (ImGui.BeginPopup("##scene_bg_ctx"))
        {
            if (ImGui.BeginMenu("Add Entity"))
            {
                if (ImGui.MenuItem("Camera"))
                {
                    var world = _sceneManager.ActiveWorld;
                    if (world != null)
                        EntityFactory.CreateCamera(world);
                }
                if (ImGui.MenuItem("Sprite"))
                {
                    var world = _sceneManager.ActiveWorld;
                    if (world != null)
                        EntityFactory.CreateSprite(world);
                }
                ImGui.EndMenu();
            }

            ImGui.EndPopup();
        }
    }

    // ── 单个节点绘制 ──

    private void DrawNode(HierarchyNode node)
    {
        bool hasChildren = node.ChildCount > 0;
        bool expanded = _cache.IsExpanded(node.Entity);
        bool selected = _cache.IsSelected(node.Entity);

        // 缩进（按 depth）
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

        //string label = $"{node.Name}##{node.Entity}";
        string label = $"{node.Entity}";
        bool nowOpen = ImGui.TreeNodeEx(label, flags);

        // 展开状态变化 → 同步缓存
        if (hasChildren && nowOpen != expanded)
            _cache.SetExpanded(node.Entity, nowOpen);

        // 点击选中（排除展开/折叠操作）
        if (ImGui.IsItemClicked(ImGuiMouseButton.Left) && !ImGui.IsItemToggledOpen())
        {
            _cache.Select(node.Entity, ImGui.GetIO().KeyCtrl);
            // 发出 SelectionChanged 事件，通知 DetailInspector 等面板刷新
            _eventBus?.Emit(new EditorEvent(
                EditorEventType.SelectionChanged, node.Entity));
        }

        // ── 拖放源 ──
        if (ImGui.BeginDragDropSource())
        {
            unsafe
            {
                ulong entityHandle = node.Entity;
                ImGui.SetDragDropPayload("ENTITY", (void*)&entityHandle, (nuint)sizeof(ulong));
            }
            ImGui.Text(node.Name);
            ImGui.EndDragDropSource();
        }

        // ── 拖放目标（重设 Parent）──
        if (hasChildren && ImGui.BeginDragDropTarget())
        {
            var payload = ImGui.AcceptDragDropPayload("ENTITY");
            if (!payload.IsNull)
            {
                unsafe
                {
                    ulong dragged = *(ulong*)payload.Data;
                    if (dragged != node.Entity)
                    {
                        // TODO: SceneNativeBridge.SetParent(_sceneHandle, dragged, node.Entity);
                        // 下一帧版本检查 → 自动刷新
                    }
                }
            }
            ImGui.EndDragDropTarget();
        }

        // ── 右键菜单 ──
        if (ImGui.BeginPopupContextItem($"##ctx_{node.Entity}"))
        {
            ImGui.Text(node.Name);
            ImGui.Separator();
            if (ImGui.MenuItem("Rename"))
                BeginRename(node);
            if (ImGui.MenuItem("Duplicate"))
                DuplicateEntity(node);
            if (ImGui.MenuItem("Delete"))
                DeleteEntity(node);
            ImGui.EndPopup();
        }

        // ── 重命名弹窗 ──
        DrawRenamePopup();

        if (nowOpen && hasChildren)
            ImGui.TreePop();
    }

    // ── 实体操作 ──

    private void BeginRename(HierarchyNode node)
    {
        _renamingEntity = node.Entity;
        _renameBuf = node.Name;
        ImGui.OpenPopup("##rename_popup");
    }

    private void DrawRenamePopup()
    {
        if (_renamingEntity == 0) return;

        bool open = true;
        if (ImGui.BeginPopupModal("##rename_popup", ref open, ImGuiWindowFlags.AlwaysAutoResize))
        {
            ImGui.Text("Rename Entity");
            ImGui.Separator();

            bool enterPressed = ImGui.InputText("##rename_input", ref _renameBuf, 256,
                ImGuiInputTextFlags.EnterReturnsTrue);

            if (ImGui.Button("OK") || enterPressed)
            {
                ApplyRename();
                ImGui.CloseCurrentPopup();
            }

            ImGui.SameLine();
            if (ImGui.Button("Cancel"))
            {
                _renamingEntity = 0;
                ImGui.CloseCurrentPopup();
            }

            ImGui.EndPopup();
        }
    }

    private void ApplyRename()
    {
        if (_renamingEntity == 0 || string.IsNullOrWhiteSpace(_renameBuf))
        {
            _renamingEntity = 0;
            return;
        }

        var world = _sceneManager.ActiveWorld;
        if (world == null)
        {
            _renamingEntity = 0;
            return;
        }

        var entity = world.Entities.Find(new NNEntityHandle(_renamingEntity));
        if (entity != null)
        {
            entity.DisplayName = _renameBuf.Trim();
        }

        _renamingEntity = 0;
    }

    private void DeleteEntity(HierarchyNode node)
    {
        var world = _sceneManager.ActiveWorld;
        if (world == null) return;

        var entity = world.Entities.Find(new NNEntityHandle(node.Entity));
        if (entity != null)
            world.DestroyEntity(entity);
    }

    private void DuplicateEntity(HierarchyNode node)
    {
        var world = _sceneManager.ActiveWorld;
        if (world == null) return;

        var source = world.Entities.Find(new NNEntityHandle(node.Entity));
        if (source == null) return;

        var duplicate = world.CreateEntity(source.DisplayName + " (Copy)");
        if (duplicate == null) return;

        // 复制 Transform 组件
        if (source.HasComponent<NNTransformData>())
        {
            var transform = source.GetComponent<NNTransformData>();
            if (transform != null)
            {
                duplicate.AddComponent<NNTransformData>();
                duplicate.SetComponent(transform.Value);
            }
        }

        // 复制 Camera 组件
        if (source.HasComponent<NNCameraComponentData>())
        {
            var camera = source.GetComponent<NNCameraComponentData>();
            if (camera != null)
            {
                duplicate.AddComponent<NNCameraComponentData>();
                duplicate.SetComponent(camera.Value);
            }
        }
    }
}
