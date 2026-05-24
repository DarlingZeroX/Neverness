using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Private.Panel;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.Framework.Private;

public static class EditorFrameworkModuleImp
{
    /// <summary>
    /// 将模块挂载到编辑器中。
    /// 仅注册基础设施：主窗口、菜单栏、Toolbar 按钮。
    /// Feature 面板由各 Feature 模块自行注册。
    /// </summary>
    public static void Install(Window window)
    {
        // 注册 Toolbar 按钮
        ImGuiToolbarRenderer.Register(new ToolbarCommand("save", FontAwesome5Pro.Save, "Save Scene", "file.save", 100));
        ImGuiToolbarRenderer.Register(new ToolbarCommand("play", FontAwesome5Pro.Play, "Play", "game.play", 200));
        ImGuiToolbarRenderer.Register(new ToolbarCommand("stop", FontAwesome5Pro.Stop, "Stop", "game.stop", 300));

        Panel.Main.EditorMainWindow mainWindow = new Panel.Main.EditorMainWindow();

        PanelManager.Instance.AddPanelWithID("EditorMainWindow", mainWindow);
        PanelManager.Instance.AddPanelWithID("EditorMenuBar", new Panel.Main.EditorMenuBar(window));
    }

    public static void TickEditorUI()
    {
        PanelManager.Instance.OnGUI();
        PanelManager.Instance.OnUpdate(0f);
    }
}
