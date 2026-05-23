using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 菜单注册表公开 API——模块通过此类贡献主菜单和上下文菜单。
/// 所有方法委托至内部 MenuRegistryImp / ContextMenuManager 单例。
/// </summary>
public static class EditorMenuRegistry
{
    // ================= 主菜单 =================

    /// <summary>注册菜单项。</summary>
    public static void Register(EditorMenuItem item) =>
        MenuRegistryImp.Instance.Register(item);

    /// <summary>批量注册菜单项。</summary>
    public static void RegisterAll(ReadOnlySpan<EditorMenuItem> items) =>
        MenuRegistryImp.Instance.RegisterAll(items);

    /// <summary>注册动态菜单项供应器。</summary>
    public static void RegisterDynamic(string menuPath, Action<DynamicMenuBuilder> builder) =>
        MenuRegistryImp.Instance.RegisterDynamic(menuPath, builder);

    /// <summary>注册命令到全局命令表。</summary>
    public static void RegisterCommand(EditorCommand command) =>
        MenuRegistryImp.Instance.RegisterCommand(command);

    /// <summary>执行命令（按 Id 查找并执行）。</summary>
    public static bool ExecuteCommand(string commandId) =>
        MenuRegistryImp.Instance.ExecuteCommand(commandId);

    /// <summary>注册菜单贡献者（插件入口）。</summary>
    public static void RegisterContributor(IMenuContributor contributor) =>
        MenuRegistryImp.Instance.RegisterContributor(contributor);

    /// <summary>移除以指定前缀路径注册的所有菜单项。</summary>
    public static void UnregisterByPrefix(string pathPrefix) =>
        MenuRegistryImp.Instance.UnregisterByPrefix(pathPrefix);

    // ================= 上下文菜单 =================

    /// <summary>注册上下文菜单贡献者（插件入口）。</summary>
    public static void RegisterContextMenuContributor(IContextMenuContributor contributor) =>
        ContextMenuManager.Instance.RegisterContributor(contributor);

    /// <summary>注册静态上下文菜单项。</summary>
    /// <param name="contextId">上下文标识符（如 "content_browser.background"）。</param>
    /// <param name="item">菜单项。</param>
    public static void RegisterContextMenuItem(string contextId, EditorMenuItem item) =>
        ContextMenuManager.Instance.RegisterItem(contextId, item);

    /// <summary>批量注册静态上下文菜单项。</summary>
    public static void RegisterContextMenuItems(string contextId, ReadOnlySpan<EditorMenuItem> items) =>
        ContextMenuManager.Instance.RegisterItems(contextId, items);

    /// <summary>注册回调式动态上下文菜单（每次弹出时调用）。</summary>
    public static void RegisterContextMenuCallback(string contextId, Action<ContextMenuBuilder> builder) =>
        ContextMenuManager.Instance.RegisterCallback(contextId, builder);

    /// <summary>清空指定上下文的所有菜单项。</summary>
    public static void ClearContextMenu(string contextId) =>
        ContextMenuManager.Instance.Clear(contextId);
}
