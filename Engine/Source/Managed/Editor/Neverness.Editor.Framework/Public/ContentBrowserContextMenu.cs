using Neverness.Editor.Framework.Private.Menu;

namespace Neverness.Editor.Framework.Public;

/// <summary>
/// ContentBrowser 上下文菜单公开 API——外部插件通过此类扩展内容浏览器的右键菜单。
/// </summary>
/// <remarks>
/// 外部扩展示例：
/// <code>
/// // 方式一：注册贡献者（推荐，批量注册）
/// EditorMenuRegistry.RegisterContextMenuContributor(new MyContentBrowserContributor());
///
/// // 方式二：直接注册单个菜单项
/// EditorMenuRegistry.RegisterContextMenuItem(
///     ContentBrowserContextMenu.BackgroundId,
///     new EditorMenuItem("My Action",
///         Command: new EditorCommand { ... },
///         Icon: FontAwesome5Pro.Plus, SortOrder: 800));
///
/// // 方式三：注册动态回调
/// EditorMenuRegistry.RegisterContextMenuCallback(
///     ContentBrowserContextMenu.ItemId,
///     builder => builder.Add("Dynamic Action", () => { ... }));
/// </code>
/// </remarks>
public static class ContentBrowserContextMenu
{
    // ================= Context ID =================

    /// <summary>空白区域右键上下文 ID。</summary>
    public const string BackgroundId = "content_browser.background";

    /// <summary>项目（文件/目录）右键上下文 ID。</summary>
    public const string ItemId = "content_browser.item";

    // ================= 运行时上下文键 =================

    /// <summary>当前目录路径（string，Background 菜单可用）。</summary>
    public const string KeyPath = "path";

    /// <summary>ContentBrowser 实例（Private.Core.ContentBrowser，两个菜单均可用）。</summary>
    public const string KeyContentBrowser = "contentBrowser";

    /// <summary>当前右键的 ContentItem（Private.Core.ContentItem，Item 菜单可用）。</summary>
    public const string KeyItem = "item";

    // ================= 便捷注册方法 =================

    /// <summary>向空白区域右键菜单注册一个菜单项。</summary>
    public static void AddBackgroundItem(EditorMenuItem item) =>
        ContextMenuManager.Instance.RegisterItem(BackgroundId, item);

    /// <summary>向项目右键菜单注册一个菜单项。</summary>
    public static void AddItem(EditorMenuItem item) =>
        ContextMenuManager.Instance.RegisterItem(ItemId, item);

    /// <summary>向空白区域右键菜单注册回调式动态菜单项。</summary>
    public static void AddBackgroundCallback(Action<ContextMenuBuilder> builder) =>
        ContextMenuManager.Instance.RegisterCallback(BackgroundId, builder);

    /// <summary>向项目右键菜单注册回调式动态菜单项。</summary>
    public static void AddItemCallback(Action<ContextMenuBuilder> builder) =>
        ContextMenuManager.Instance.RegisterCallback(ItemId, builder);
}
