using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Private.Core;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Private.Panel;
using Neverness.Editor.Framework.Private.Panel.ContentBrowser;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Application.Public;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Framework.Private;

public static class EditorFrameworkModuleImp
{
    /// <summary>
    /// 将模块挂载到编辑器中。
    /// </summary>
    public static void Install(Window window)
    {
        string? path = VFS.GetAbsolutePath(ProjectPaths.Assets);
        if (path != null)
        {
            ContentBrowser.Create(path);
        }
        else
        {
            throw new InvalidOperationException("Failed to get absolute path for assets directory.");
        }

        // 注册主菜单贡献者
        EditorMenuRegistry.RegisterContributor(new BuiltinMenuContributor());

        // 注册上下文菜单贡献者
        EditorMenuRegistry.RegisterContextMenuContributor(new ContentBrowserContextMenuContributor());

        // 注册 Toolbar 按钮
        ImGuiToolbarRenderer.Register(new ToolbarCommand("save", FontAwesome5Pro.Save, "Save Scene", "file.save", 100));
        ImGuiToolbarRenderer.Register(new ToolbarCommand("play", FontAwesome5Pro.Play, "Play", "game.play", 200));
        ImGuiToolbarRenderer.Register(new ToolbarCommand("stop", FontAwesome5Pro.Stop, "Stop", "game.stop", 300));

        //ImGui.ImGui()
        Panel.Main.EditorMainWindow mainWindow = new Panel.Main.EditorMainWindow();

        PanelManager.Instance.AddPanelWithID("EditorMainWindow", mainWindow);
        PanelManager.Instance.AddPanelWithID("EditorMenuBar", new Panel.Main.EditorMenuBar(window));

        mainWindow.AddPanelWithID("EditorViewport", new EditorViewport());
        mainWindow.AddPanelWithID("ContentBrowser", new ContentBrowserPanel());
        mainWindow.AddPanelWithID("SceneBrowser", new SceneBrowser());
    }

    public static void TickEditorUI()
    {
        // 实现逻辑
        PanelManager.Instance.OnGUI();
    }
}
