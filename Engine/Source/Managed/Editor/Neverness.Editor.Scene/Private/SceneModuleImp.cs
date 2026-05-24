using Neverness.Editor.Framework.Private;
using Neverness.Runtime.Scene;
using Neverness.Editor.Scene.Private.Panel;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// 场景编辑模块内部安装实现。
/// 负责：场景浏览器面板、编辑器视口面板。
/// </summary>
internal static class SceneModuleImp
{
    private static SceneBrowser? s_sceneBrowser;
    private static SceneEditorBridge? s_bridge;

    /// <summary>安装场景编辑模块（场景句柄后续设置）。</summary>
    public static void Install(SceneManager sceneManager)
    {
        s_sceneBrowser = new SceneBrowser(sceneManager);
        PanelManager.Instance.AddChildPanel("SceneBrowser", s_sceneBrowser);
        PanelManager.Instance.AddChildPanel("EditorViewport", new EditorViewport());

        // 创建场景编辑器桥接（事件驱动）
        s_bridge = new SceneEditorBridge(sceneManager);
    }

    /// <summary>设置场景浏览器关联的场景句柄。</summary>
    public static void SetSceneHandle(ulong sceneHandle)
    {
        if (s_sceneBrowser != null)
            s_sceneBrowser.SceneHandle = sceneHandle;
    }
}
