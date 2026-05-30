using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.Shell.Public;

/// <summary>
/// Shell 模块公开 API——负责编辑器主窗口和菜单栏的安装。
/// </summary>
public static class ShellModule
{
    /// <summary>
    /// 安装主窗口和菜单栏。
    /// 必须在 Framework.Install() 之后、Core.Install() 之前调用。
    /// </summary>
    public static void Install(Window window)
    {
        Private.ShellModuleImp.Install(window);
    }
}
