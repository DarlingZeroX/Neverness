using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Public;

/// <summary>
/// Neverness.Editor.Scene 模块安装入口。
/// 编辑器启动时调用 <see cref="Install"/> 完成场景编辑功能的初始化。
/// </summary>
public static class SceneModule
{
    /// <summary>安装场景编辑模块（场景句柄后续设置）。</summary>
    public static void Install(SceneManager sceneManager)
    {
        Private.SceneModuleImp.Install(sceneManager);
    }

    /// <summary>设置场景浏览器关联的场景句柄。</summary>
    public static void SetSceneHandle(ulong sceneHandle)
    {
        Private.SceneModuleImp.SetSceneHandle(sceneHandle);
    }
}
