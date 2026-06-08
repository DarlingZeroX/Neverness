using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 上下文菜单管理——支持静态项、回调式动态菜单、贡献者模式。
/// 可泛化：任何面板/区域通过 contextId 注册和渲染各自的右键菜单。
///
/// 注意：此类只负责注册逻辑（UI 无关）。
/// 渲染逻辑由 <see cref="IContextMenuRenderer"/> 实现（ImGuiFrontend 提供）。
/// </summary>
public sealed class ContextMenuManager : IContextMenuRegistry
{
    /// <summary>全局单例。</summary>
    public static ContextMenuManager Instance { get; } = new();

    /// <summary>
    /// 渲染器实例——由 ImGuiFrontend 在启动时注入。
    /// 旧代码通过此类的方法间接调用渲染器，无需直接引用 ImGui。
    /// </summary>
    public static IContextMenuRenderer? Renderer { get; set; }

    private readonly Dictionary<string, List<EditorMenuItem>> _staticMenus = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, List<Action<ContextMenuBuilder>>> _callbacks = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<IContextMenuContributor> _contributors = [];
    private bool _contributorsDirty = true;

    /// <summary>运行时上下文字典——渲染时注入，供命令回调读取。</summary>
    private readonly Dictionary<string, object?> _contextValues = new(StringComparer.OrdinalIgnoreCase);

    private ContextMenuManager() { }

    // ================= 贡献者管理 =================

    /// <summary>注册上下文菜单贡献者（插件入口）。</summary>
    public void RegisterContributor(IContextMenuContributor contributor)
    {
        _contributors.Add(contributor);
        _contributorsDirty = true;
    }

    /// <summary>确保所有贡献者已构建。</summary>
    public void EnsureContributors()
    {
        if (!_contributorsDirty) return;
        foreach (var c in _contributors)
            c.Build(this);
        _contributorsDirty = false;
    }

    // ================= 运行时上下文 =================

    /// <summary>设置运行时上下文值（渲染前调用）。</summary>
    public void SetContext(string key, object? value) => _contextValues[key] = value;

    /// <summary>获取运行时上下文值。</summary>
    public object? GetContext(string key) =>
        _contextValues.TryGetValue(key, out var v) ? v : null;

    /// <summary>获取运行时上下文值（强类型）。</summary>
    public T? GetContext<T>(string key) where T : class =>
        GetContext(key) as T;

    /// <summary>清空运行时上下文（渲染后调用）。</summary>
    public void ClearContext() => _contextValues.Clear();

    // ================= 静态项注册 =================

    /// <summary>注册静态上下文菜单项。</summary>
    public void RegisterItem(string contextId, EditorMenuItem item)
    {
        if (!_staticMenus.TryGetValue(contextId, out var list))
        {
            list = [];
            _staticMenus[contextId] = list;
        }
        list.Add(item);
    }

    /// <summary>批量注册静态上下文菜单项。</summary>
    public void RegisterItems(string contextId, ReadOnlySpan<EditorMenuItem> items)
    {
        foreach (ref readonly var item in items)
            RegisterItem(contextId, item);
    }

    /// <summary>注册回调式动态上下文菜单（每次弹出时调用）。</summary>
    public void RegisterCallback(string contextId, Action<ContextMenuBuilder> builder)
    {
        if (!_callbacks.TryGetValue(contextId, out var list))
        {
            list = [];
            _callbacks[contextId] = list;
        }
        list.Add(builder);
    }

    // ================= 数据访问（供 IContextMenuRenderer 渲染使用） =================

    /// <summary>获取指定 contextId 的静态菜单项列表。</summary>
    public IReadOnlyList<EditorMenuItem>? GetStaticItems(string contextId) =>
        _staticMenus.TryGetValue(contextId, out var items) ? items : null;

    /// <summary>获取指定 contextId 的回调列表。</summary>
    public IReadOnlyList<Action<ContextMenuBuilder>>? GetCallbacks(string contextId) =>
        _callbacks.TryGetValue(contextId, out var callbacks) ? callbacks : null;

    /// <summary>清空指定上下文的所有菜单项。</summary>
    public void Clear(string contextId)
    {
        _staticMenus.Remove(contextId);
        _callbacks.Remove(contextId);
    }

    // ================= 渲染委托（兼容旧代码） =================

    /// <summary>开始窗口级弹出菜单（委托给 Renderer）。</summary>
    public bool BeginWindowPopup(string contextId) =>
        Renderer?.BeginWindowPopup(contextId) ?? false;

    /// <summary>开始项目级弹出菜单（委托给 Renderer）。</summary>
    public bool BeginItemPopup(string contextId) =>
        Renderer?.BeginItemPopup(contextId) ?? false;

    /// <summary>结束弹出菜单（委托给 Renderer）。</summary>
    public void EndPopup() => Renderer?.EndPopup();

    /// <summary>在弹出菜单内渲染所有已注册的内容（委托给 Renderer）。</summary>
    public void RenderPopupContent(string contextId) =>
        Renderer?.RenderPopupContent(contextId, this);

    /// <summary>完整渲染窗口级上下文菜单（委托给 Renderer）。</summary>
    public void RenderWindowContextMenu(string contextId) =>
        Renderer?.RenderWindowContextMenu(contextId, this);

    /// <summary>完整渲染项目级上下文菜单（委托给 Renderer）。</summary>
    public void RenderItemContextMenu(string contextId) =>
        Renderer?.RenderItemContextMenu(contextId, this);
}

/// <summary>
/// 上下文菜单构建器——回调中使用，用于动态生成上下文菜单项。
/// </summary>
public sealed class ContextMenuBuilder
{
    private readonly List<EditorMenuItem> _items = [];

    /// <summary>添加菜单项。</summary>
    public ContextMenuBuilder Add(string label, Action execute, string icon = "", string shortcut = "")
    {
        _items.Add(new EditorMenuItem(
            Path: label,
            Command: new EditorCommand
            {
                Id = "ctx." + label.Replace(" ", "_").Replace(".", "_"),
                DisplayName = label,
                Execute = _ => execute(),
            },
            Icon: icon,
            Shortcut: shortcut
        ));
        return this;
    }

    /// <summary>添加带 CanExecute 条件的菜单项。</summary>
    public ContextMenuBuilder Add(string label, Action execute, Func<bool> canExecute, string icon = "")
    {
        _items.Add(new EditorMenuItem(
            Path: label,
            Command: new EditorCommand
            {
                Id = "ctx." + label.Replace(" ", "_").Replace(".", "_"),
                DisplayName = label,
                Execute = _ => execute(),
                CanExecute = canExecute,
            },
            Icon: icon
        ));
        return this;
    }

    /// <summary>添加分隔符。</summary>
    public ContextMenuBuilder Separator()
    {
        _items.Add(new EditorMenuItem(Path: "sep", IsSeparator: true));
        return this;
    }

    /// <summary>构建结果列表。</summary>
    public IReadOnlyList<EditorMenuItem> Build() => _items;
}
