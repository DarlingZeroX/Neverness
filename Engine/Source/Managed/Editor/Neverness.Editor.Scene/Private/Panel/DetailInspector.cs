using Hexa.NET.ImGui;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.Cache;
using Neverness.Editor.Scene.Private.Inspector;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Internal;
using System.Numerics;
using Neverness.Editor.Scene.Private.Debug;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// Detail Inspector 面板——Unity 风格的实体组件查看/编辑器。
///
/// 数据流：
///   SceneBrowser 选中实体 → EditorEventBus.SelectionChanged 事件
///   → DetailInspector 监听事件 → 刷新选中实体 → 枚举组件 → 绘制 Inspector
///
/// 架构：
///   组件绘制由 <see cref="ComponentInspectorRegistry"/> 分发，
///   每种组件类型（Transform、Camera 等）注册独立的 <see cref="IComponentInspector"/>。
///   新增组件类型只需添加新的 Inspector 实现并注册，无需修改 DetailInspector 代码。
/// </summary>
public sealed class DetailInspector : IEditorPanel
{
    private bool _isOpen = true;
    private readonly SceneManager _sceneManager;

    /// <summary>当前选中的实体句柄值（0 = 无选中）。</summary>
    private ulong _selectedEntityHandle;

    /// <summary>当前选中实体的显示名称缓存。</summary>
    private string _selectedEntityName = "";

    /// <summary>关联的场景句柄（由 SceneModule 设置）。</summary>
    private ulong _sceneHandle;

    /// <summary>关联的场景层级缓存引用（用于获取选中状态）。</summary>
    private SceneHierarchyCache? _hierarchyCache;

    /// <summary>是否已订阅事件总线。</summary>
    private bool _eventsSubscribed;

    public DetailInspector(SceneManager sceneManager)
    {
        _sceneManager = sceneManager;
    }

    /// <summary>设置或获取当前关联的场景句柄。</summary>
    public ulong SceneHandle
    {
        get => _sceneHandle;
        set => _sceneHandle = value;
    }

    /// <summary>设置场景层级缓存引用（用于直接查询选中状态）。</summary>
    public SceneHierarchyCache? HierarchyCache
    {
        get => _hierarchyCache;
        set => _hierarchyCache = value;
    }

    // ── IEditorPanel ──

    public void OnUpdate(float delta)
    {
        EnsureEventsSubscribed();
        SyncSelection();
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

        if (_selectedEntityHandle == 0)
        {
            DrawEmptyState();
            ImGui.End();
            return;
        }

        DrawEntityHeader();
        DrawComponents();
        DrawAddComponentButton();

        ImGui.End();
    }

    public string GetWindowFullName() => FontAwesome5Pro.InfoCircle + " " + GetWindowName();

    public string GetWindowName() => "DetailInspector";

    public void OpenWindow(bool open) => _isOpen = open;

    public bool IsWindowOpened() => _isOpen;

    // ── 事件订阅 ──

    /// <summary>确保已订阅编辑器事件总线。延迟到首次 OnUpdate 以确保 EditorCoreModule 已安装。</summary>
    private void EnsureEventsSubscribed()
    {
        if (_eventsSubscribed) return;

        try
        {
            var eventBus = EditorCoreModule.Context.Events;
            eventBus.Subscribe(EditorEventType.SelectionChanged, OnSelectionChanged);
            eventBus.Subscribe(EditorEventType.SceneClosed, OnSceneClosed);
            _eventsSubscribed = true;
        }
        catch (InvalidOperationException)
        {
            // EditorCoreModule 尚未安装，下一帧重试
        }
    }

    /// <summary>SelectionChanged 事件处理。</summary>
    private void OnSelectionChanged(EditorEvent evt)
    {
        if (evt.Payload is ulong handle)
        {
            SetSelectedEntity(handle);
        }
    }

    /// <summary>SceneClosed 事件处理——清空选中。</summary>
    private void OnSceneClosed(EditorEvent evt)
    {
        SetSelectedEntity(0);
    }

    // ── 选中同步 ──

    /// <summary>
    /// 同步选中状态——优先从事件总线获取，回退到直接轮询 SceneHierarchyCache。
    /// 此方法确保即使 SceneBrowser 未发出事件，Inspector 也能跟踪选中。
    /// </summary>
    private void SyncSelection()
    {
        if (_hierarchyCache == null) return;

        var selected = _hierarchyCache.SelectedEntities;
        ulong cacheSelection = 0;
        if (selected.Count > 0)
        {
            // IReadOnlyCollection 的枚举器取第一个元素
            using var enumerator = selected.GetEnumerator();
            if (enumerator.MoveNext())
                cacheSelection = enumerator.Current;
        }

        if (cacheSelection != _selectedEntityHandle)
        {
            SetSelectedEntity(cacheSelection);
        }
    }

    /// <summary>设置选中实体并刷新显示名称。</summary>
    private void SetSelectedEntity(ulong entityHandle)
    {
        _selectedEntityHandle = entityHandle;
        _selectedEntityName = "";

        if (entityHandle == 0) return;

        // 从层级缓存获取名称
        if (_hierarchyCache != null)
        {
            var node = _hierarchyCache.GetNode(entityHandle);
            if (node != null)
            {
                _selectedEntityName = node.Name;
                return;
            }
        }

        // 回退：直接使用句柄值
        _selectedEntityName = $"Entity ({entityHandle})";
    }

    /// <summary>清除选中状态（场景切换时由 SceneModuleImp 调用）。</summary>
    internal void ClearSelection()
    {
        _selectedEntityHandle = 0;
        _selectedEntityName = "";
    }

    // ── UI 绘制 ──

    /// <summary>绘制未选中状态的空面板。</summary>
    private static void DrawEmptyState()
    {
        var cursor = ImGui.GetCursorPos();
        var available = ImGui.GetContentRegionAvail();

        var text = "No entity selected.";
        var textSize = ImGui.CalcTextSize(text);

        ImGui.SetCursorPos(new Vector2(
            cursor.X + (available.X - textSize.X) * 0.5f,
            cursor.Y + available.Y * 0.3f));
        ImGui.TextDisabled(text);

        var subText = "Select an entity in the Scene Browser.";
        var subTextSize = ImGui.CalcTextSize(subText);
        ImGui.SetCursorPos(new Vector2(
            cursor.X + (available.X - subTextSize.X) * 0.5f,
            cursor.Y + available.Y * 0.3f + textSize.Y + 8f));
        ImGui.TextDisabled(subText);
    }

    /// <summary>绘制实体信息头部（名称、句柄、Active 复选框）。</summary>
    private void DrawEntityHeader()
    {
        // 实体名称 + 句柄
        ImGui.Text(_selectedEntityName);
        ImGui.SameLine();
        ImGui.TextDisabled($"  (0x{_selectedEntityHandle:X16})");

        // Active 状态
        ImGui.Spacing();

        // 从层级缓存读取 active 状态
        bool isActive = true;
        if (_hierarchyCache != null)
        {
            var node = _hierarchyCache.GetNode(_selectedEntityHandle);
            if (node != null)
                isActive = node.IsActive;
        }

        bool active = isActive;
        if (ImGui.Checkbox("Active", ref active))
        {
            // TODO: 通过 Native API 设置实体 Active 状态
        }

        ImGui.Separator();
    }

    /// <summary>遍历已注册的组件检查器，绘制选中实体拥有的每个组件。</summary>
    private void DrawComponents()
    {
        var inspectors = ComponentInspectorRegistry.Inspectors;
        var handle = new NNEntityHandle(_selectedEntityHandle);

        for (int i = 0; i < inspectors.Count; i++)
        {
            var inspector = inspectors[i];

            if (!inspector.HasComponent(_sceneHandle, handle))
                continue;

            DrawComponentSection(inspector, handle);
        }

        //SceneDebug.DumpEntityComponents(_sceneHandle, handle.Value);
    }

    /// <summary>绘制单个组件的折叠面板（标题栏 + 字段编辑区 + 右键菜单）。</summary>
    private void DrawComponentSection(IComponentInspector inspector, NNEntityHandle entity)
    {
        // PushID 避免 ImGui ID 冲突
        ImGui.PushID((int)(inspector.ComponentTypeId & 0x7FFFFFFF));

        // 折叠面板头（Unity 风格：组件名 + 右键齿轮菜单）
        bool open = ImGui.CollapsingHeader(inspector.DisplayName, ImGuiTreeNodeFlags.DefaultOpen);

        // 右键菜单：移除组件 / 重置
        if (ImGui.BeginPopupContextItem("##comp_ctx"))
        {
            if (ImGui.MenuItem("Remove Component"))
            {
                inspector.RemoveComponent(_sceneHandle, entity);
                ImGui.EndPopup();
                ImGui.PopID();
                return;
            }
            ImGui.EndPopup();
        }

        // 展开时绘制字段
        if (open)
        {
            ImGui.Indent(12f);
            bool modified = inspector.DrawInspector(_sceneHandle, entity);
            ImGui.Unindent(12f);

            // 修改后可在此触发额外逻辑（如标记场景 dirty）
            if (modified)
            {
                // TODO: 通知场景有未保存修改
            }
        }

        ImGui.PopID();
    }

    /// <summary>绘制"添加组件"按钮和弹窗。</summary>
    private void DrawAddComponentButton()
    {
        ImGui.Spacing();

        // 居中按钮
        float availWidth = ImGui.GetContentRegionAvail().X;
        float buttonWidth = 200f;
        ImGui.SetCursorPosX(ImGui.GetCursorPosX() + (availWidth - buttonWidth) * 0.5f);

        if (ImGui.Button(FontAwesome5Pro.Plus + " Add Component", new Vector2(buttonWidth, 0)))
        {
            ImGui.OpenPopup("##add_component_popup");
        }

        DrawAddComponentPopup();
    }

    /// <summary>绘制"添加组件"弹窗——列出所有已注册但当前实体尚未拥有的组件。</summary>
    private void DrawAddComponentPopup()
    {
        if (!ImGui.BeginPopup("##add_component_popup"))
            return;

        var inspectors = ComponentInspectorRegistry.Inspectors;
        var handle = new NNEntityHandle(_selectedEntityHandle);
        bool hasAny = false;

        for (int i = 0; i < inspectors.Count; i++)
        {
            var inspector = inspectors[i];

            // 跳过已有组件
            if (inspector.HasComponent(_sceneHandle, handle))
                continue;

            hasAny = true;
            if (ImGui.MenuItem(inspector.DisplayName))
            {
                // 通过 Native API 添加组件（使用 TypeId 的非泛型重载）
                SceneNativeBridge.AddComponent(_sceneHandle, handle, inspector.ComponentTypeId);
            }
        }

        if (!hasAny)
        {
            ImGui.TextDisabled("No more components available.");
        }

        ImGui.EndPopup();
    }
}
