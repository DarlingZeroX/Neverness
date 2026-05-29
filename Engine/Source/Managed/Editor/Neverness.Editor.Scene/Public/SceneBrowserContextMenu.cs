namespace Neverness.Editor.Scene.Public;

/// <summary>
/// SceneBrowser 上下文菜单公开 API——外部插件通过此类扩展场景浏览器的右键菜单。
/// </summary>
/// <remarks>
/// 外部扩展示例：
/// <code>
/// // 注册贡献者（推荐）
/// EditorMenuRegistry.RegisterContextMenuContributor(new MySceneContributor());
///
/// // 或直接注册单个菜单项
/// EditorMenuRegistry.RegisterContextMenuItem(
///     SceneBrowserContextMenu.BackgroundId,
///     new EditorMenuItem("My Entity",
///         Command: new EditorCommand { ... },
///         SortOrder: 300));
/// </code>
/// </remarks>
public static class SceneBrowserContextMenu
{
    /// <summary>空白区域右键菜单 context ID。</summary>
    public const string BackgroundId = "scene_browser.background";

    /// <summary>当前活动世界 context key（类型 <c>SceneWorld</c>）。</summary>
    public const string KeyActiveWorld = "activeWorld";
}
