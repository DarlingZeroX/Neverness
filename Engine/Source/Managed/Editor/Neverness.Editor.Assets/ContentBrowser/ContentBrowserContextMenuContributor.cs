using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Assets.Private.Core;

// 图标使用 emoji（Avalonia 无法加载 FontAwesome 字体）
// FolderPlus→📁, Redo→↻, Copy→📋, ExternalLinkAlt→📂, Pen→✏️, Trash→🗑️

namespace Neverness.Editor.Assets.Private.Context;

/// <summary>
/// 内容浏览器上下文菜单贡献者——注册内置的右键菜单项。
/// 外部插件可通过 <see cref="ContentBrowserContextMenu"/> 或
/// <see cref="EditorMenuRegistry.RegisterContextMenuContributor"/> 扩展。
/// </summary>
public sealed class ContentBrowserContextMenuContributor : IContextMenuContributor
{
    public void Build(ContextMenuManager ctx)
    {
        RegisterBackgroundMenu(ctx);
        RegisterItemMenu(ctx);
    }

    /// <summary>注册空白区域右键菜单项。</summary>
    private static void RegisterBackgroundMenu(ContextMenuManager ctx)
    {
        var id = ContentBrowserContextMenu.BackgroundId;

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Create Directory",
            Command: new EditorCommand
            {
                Id = "content_browser.create_directory",
                DisplayName = "Create Directory",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var path = ContextMenuManager.Instance.GetContext<string>(ContentBrowserContextMenu.KeyPath);
                    if (cb != null && path != null)
                        cb.CreateNewDirectory(path);
                },
            },
            Icon: "📁",
            SortOrder: 100));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Show in Explorer",
            Command: new EditorCommand
            {
                Id = "content_browser.show_in_explorer_bg",
                DisplayName = "Show in Explorer",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var path = ContextMenuManager.Instance.GetContext<string>(ContentBrowserContextMenu.KeyPath);
                    if (cb != null && path != null)
                        cb.ShowInExplorer(path);
                },
            },
            Icon: "📂",
            SortOrder: 200));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Refresh",
            Command: new EditorCommand
            {
                Id = "content_browser.refresh",
                DisplayName = "Refresh",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    cb?.RefreshDirectory();
                },
            },
            Icon: "↻",
            SortOrder: 300));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Copy Path",
            Command: new EditorCommand
            {
                Id = "content_browser.copy_path",
                DisplayName = "Copy Path",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var path = ContextMenuManager.Instance.GetContext<string>(ContentBrowserContextMenu.KeyPath);
                    if (cb != null && path != null)
                        cb.CopyPath(path);
                },
            },
            Icon: "📋",
            SortOrder: 400));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "sep1",
            IsSeparator: true,
            SortOrder: 500));

        // "Create Scene" 已迁移至 Neverness.Editor.Assets.AssetCreationMenuContributor
        // 由 AssetFactoryRegistry 动态生成所有资产类型的创建菜单项
    }

    /// <summary>注册项目右键菜单项。</summary>
    private static void RegisterItemMenu(ContextMenuManager ctx)
    {
        var id = ContentBrowserContextMenu.ItemId;

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Rename",
            Command: new EditorCommand
            {
                Id = "content_browser.rename",
                DisplayName = "Rename",
                Execute = _ =>
                {
                    var item = ContextMenuManager.Instance.GetContext<ContentItem>(ContentBrowserContextMenu.KeyItem);
                    if (item != null)
                    {
                        item.Renaming = true;
                        // 通过回调触发内联重命名 UI（由 AvaloniaFrontend 注册）
                        var beginRename = ContextMenuManager.Instance.GetContext<Action<string, string>>("content_browser.begin_rename");
                        if (beginRename != null)
                            beginRename(item.SystemPath.FullPath, item.Name);
                    }
                },
            },
            Icon: "✏️",
            SortOrder: 100));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Remove",
            Command: new EditorCommand
            {
                Id = "content_browser.remove",
                DisplayName = "Remove",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var item = ContextMenuManager.Instance.GetContext<ContentItem>(ContentBrowserContextMenu.KeyItem);
                    if (cb != null && item != null)
                        cb.DeleteDirectoryItem(item);
                },
            },
            Icon: "🗑️",
            SortOrder: 200));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Show in Explorer",
            Command: new EditorCommand
            {
                Id = "content_browser.show_in_explorer_item",
                DisplayName = "Show in Explorer",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var item = ContextMenuManager.Instance.GetContext<ContentItem>(ContentBrowserContextMenu.KeyItem);
                    if (cb != null && item != null)
                        cb.ShowInExplorer(item.SystemPath.ToString());
                },
            },
            Icon: "📂",
            SortOrder: 300));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Copy Asset Name",
            Command: new EditorCommand
            {
                Id = "content_browser.copy_asset_name",
                DisplayName = "Copy Asset Name",
                Execute = _ =>
                {
                    var cb = ContextMenuManager.Instance.GetContext<ContentBrowser>(ContentBrowserContextMenu.KeyContentBrowser);
                    var item = ContextMenuManager.Instance.GetContext<ContentItem>(ContentBrowserContextMenu.KeyItem);
                    if (cb != null && item != null)
                        cb.CopyPath(item.Name);
                },
            },
            Icon: "📋",
            SortOrder: 400));
    }
}
