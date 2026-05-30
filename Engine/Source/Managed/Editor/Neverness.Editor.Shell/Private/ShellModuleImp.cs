using Neverness.Editor.Framework.Private;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.Shell.Private;

/// <summary>
/// Shell 模块内部实现——创建主窗口和菜单栏，注册到 PanelManager。
/// </summary>
internal static class ShellModuleImp
{
    /// <summary>
    /// 安装主窗口和菜单栏。
    /// </summary>
    public static void Install(Window window)
    {
        var panelManager = PanelManager.Instance;

        // 创建主窗口和菜单栏
        var mainWindow = new EditorMainWindow();
        var menuBar = new EditorMenuBar(window);

        // 注册到 PanelManager
        panelManager.AddPanelWithID("EditorMainWindow", mainWindow);
        panelManager.AddPanelWithID("EditorMenuBar", menuBar);

        // 注册 AddChildPanel 回调，使 PanelManager 能委托子面板注册到主窗口
        panelManager.RegisterMainWindowCallback((id, panel) =>
        {
            mainWindow.AddPanelWithID(id, panel);
            return true;
        });
    }
}
