namespace Neverness.Editor.Core.Public;

/// <summary>
/// Neverness.Editor.Core 模块安装入口。
/// 编辑器启动时调用 <see cref="Install"/> 完成运行时中枢的初始化。
/// </summary>
public static class EditorCoreModule
{
    /// <summary>安装编辑器核心模块。</summary>
    public static void Install()
    {
        Private.CoreModuleImp.Install();
    }

    /// <summary>获取编辑器上下文（Install 后可用）。</summary>
    public static IEditorContext Context => Private.CoreModuleImp.Context;

    /// <summary>关闭编辑器核心模块。</summary>
    public static void Shutdown()
    {
        Private.CoreModuleImp.Shutdown();
    }
}
