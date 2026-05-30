namespace Neverness.Editor.Framework.Public;

public static class EditorFrameworkModule
{
    /// <summary>
    /// 将模块挂载到编辑器中。
    /// 仅注册 Toolbar 按钮等基础设施。
    /// 主窗口和菜单栏由 Shell 模块负责。
    /// </summary>
    public static void Install()
    {
        Private.EditorFrameworkModuleImp.Install();
    }

    public static void TickEditorUI()
    {
        Private.EditorFrameworkModuleImp.TickEditorUI();
    }
}
