using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.ImGuiFrontend.ContextMenus;

/// <summary>
/// ContentBrowser ImGui 上下文菜单贡献者——使用 Controller 执行操作。
/// 替代 Assets 模块的 ContentBrowserContextMenuContributor。
/// </summary>
public sealed class ContentBrowserImGuiContextMenuContributor : IContextMenuContributor
{
    private readonly ContentBrowserController _controller;

    public ContentBrowserImGuiContextMenuContributor(ContentBrowserController controller)
    {
        _controller = controller;
    }

    public void Build(ContextMenuManager ctx)
    {
        RegisterBackgroundMenu(ctx);
    }

    /// <summary>注册空白区域右键菜单项。</summary>
    private void RegisterBackgroundMenu(ContextMenuManager ctx)
    {
        var id = "content_browser.background";

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Create Directory",
            Command: new EditorCommand
            {
                Id = "content_browser.create_directory",
                DisplayName = "Create Directory",
                Execute = _ =>
                {
                    _controller.CreateNewFolder();
                },
            },
            Icon: "📁",
            SortOrder: 100));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Refresh",
            Command: new EditorCommand
            {
                Id = "content_browser.refresh",
                DisplayName = "Refresh",
                Execute = _ =>
                {
                    _controller.RefreshDirectory();
                },
            },
            Icon: "🔄",
            SortOrder: 200));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "sep1",
            IsSeparator: true,
            SortOrder: 300));

        ctx.RegisterItem(id, new EditorMenuItem(
            Path: "Show in Explorer",
            Command: new EditorCommand
            {
                Id = "content_browser.show_in_explorer",
                DisplayName = "Show in Explorer",
                Execute = _ =>
                {
                    // TODO: 打开系统文件管理器
                    var path = _controller.GetCurrentDirectory();
                    if (!string.IsNullOrEmpty(path))
                    {
                        try
                        {
                            System.Diagnostics.Process.Start("explorer.exe", $"\"{path}\"");
                        }
                        catch { }
                    }
                },
            },
            Icon: "📂",
            SortOrder: 400));
    }
}
