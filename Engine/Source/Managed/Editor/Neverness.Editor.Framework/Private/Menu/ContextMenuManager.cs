using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 上下文菜单管理——支持静态项、回调式动态菜单、贡献者模式。
/// 可泛化：任何面板/区域通过 contextId 注册和渲染各自的右键菜单。
/// </summary>
public sealed class ContextMenuManager
{
    /// <summary>全局单例。</summary>
    public static ContextMenuManager Instance { get; } = new();

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
    internal void EnsureContributors()
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

    // ================= 弹出菜单生命周期 =================

    /// <summary>开始窗口级弹出菜单（右键空白区域）。</summary>
    public bool BeginWindowPopup(string contextId) =>
        ImGui.BeginPopupContextWindow(contextId);

    /// <summary>开始项目级弹出菜单（右键某个项目）。</summary>
    public bool BeginItemPopup(string contextId) =>
        ImGui.BeginPopupContextItem(contextId);

    /// <summary>结束弹出菜单。</summary>
    public void EndPopup() => ImGui.EndPopup();

    // ================= 渲染 =================

    /// <summary>
    /// 在弹出菜单内渲染所有已注册的内容（贡献者项 + 回调项 + 静态项）。
    /// 必须在 BeginWindowPopup 或 BeginItemPopup 返回 true 后调用。
    /// </summary>
    public void RenderPopupContent(string contextId)
    {
        // 1. 贡献者注册的静态项（已构建）
        EnsureContributors();

        // 2. 回调式动态项
        if (_callbacks.TryGetValue(contextId, out var callbacks))
        {
            foreach (var cb in callbacks)
            {
                var builder = new ContextMenuBuilder();
                cb(builder);
                RenderItems(builder.Build());
            }
        }

        // 3. 静态注册的项
        if (_staticMenus.TryGetValue(contextId, out var items))
            RenderItems(items);
    }

    /// <summary>
    /// 便捷方法：完整渲染窗口级上下文菜单（Begin → 内容 → End）。
    /// 使用 NoOpenOverItems 防止在项目 hover 时误开弹窗，避免与 Item 上下文菜单冲突。
    /// </summary>
    public void RenderWindowContextMenu(string contextId)
    {
        EnsureContributors();

        if (ImGui.IsWindowHovered() && !ImGui.IsAnyItemHovered() && ImGui.IsMouseReleased(ImGuiMouseButton.Right)) 
            ImGui.OpenPopup(contextId);

        if (!ImGui.BeginPopup(contextId))
            return;
        RenderPopupContent(contextId);
        ImGui.EndPopup();
    }

    /// <summary>
    /// 便捷方法：完整渲染项目级上下文菜单（Begin → 内容 → End）。
    /// </summary>
    public void RenderItemContextMenu(string contextId)
    {
        EnsureContributors();
        if (!ImGui.BeginPopupContextItem(contextId))
            return;
        RenderPopupContent(contextId);
        ImGui.EndPopup();
    }

    /// <summary>清空指定上下文的所有菜单项。</summary>
    public void Clear(string contextId)
    {
        _staticMenus.Remove(contextId);
        _callbacks.Remove(contextId);
    }

    // ================= 内部渲染 =================

    /// <summary>渲染菜单项列表。</summary>
    private static void RenderItems(IReadOnlyList<EditorMenuItem> items)
    {
        foreach (var item in items)
        {
            if (item.IsSeparator)
            {
                ImGui.Separator();
                continue;
            }

            var label = string.IsNullOrEmpty(item.Icon)
                ? item.Command?.DisplayName ?? ""
                : item.Icon + " " + (item.Command?.DisplayName ?? "");

            bool enabled = item.Command?.CanExecute?.Invoke() ?? true;
            var selected = item.Command?.IsChecked?.Invoke() ?? false;
            var hasCheck = item.Command?.IsChecked != null;

            if (hasCheck)
            {
                if (ImGui.MenuItem(label, string.IsNullOrEmpty(item.Shortcut) ? null : item.Shortcut, selected, enabled))
                {
                    item.Command?.Execute(default);
                    ImGui.CloseCurrentPopup();
                }
            }
            else
            {
                if (ImGui.MenuItem(label, string.IsNullOrEmpty(item.Shortcut) ? null : item.Shortcut, false, enabled))
                {
                    item.Command?.Execute(default);
                    ImGui.CloseCurrentPopup();
                }
            }
        }
    }
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

    /// <summary>构建结果列表（内部使用）。</summary>
    internal IReadOnlyList<EditorMenuItem> Build() => _items;
}
