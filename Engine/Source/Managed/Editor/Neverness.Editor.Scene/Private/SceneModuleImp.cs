using Neverness.Editor.Framework.Private;
using Neverness.Runtime.Scene;
using Neverness.Editor.Scene.Private.Panel;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// 场景编辑模块内部安装实现。
/// 负责：场景浏览器面板、DetailInspector 面板、编辑器视口面板。
/// </summary>
internal static class SceneModuleImp
{
    private static SceneBrowser? s_sceneBrowser;
    private static DetailInspector? s_detailInspector;
    private static SceneEditorBridge? s_bridge;

    /// <summary>获取 SceneBrowser 的层级缓存（Debug / 诊断用）。</summary>
    public static Cache.SceneHierarchyCache? HierarchyCache => s_sceneBrowser?.Cache;

    /// <summary>安装场景编辑模块（场景句柄后续设置）。</summary>
    public static void Install(SceneManager sceneManager)
    {
        // 创建场景浏览器
        s_sceneBrowser = new SceneBrowser(sceneManager);
        PanelManager.Instance.AddChildPanel("SceneBrowser", s_sceneBrowser);

        // 创建 Detail Inspector
        s_detailInspector = new DetailInspector(sceneManager);
        PanelManager.Instance.AddChildPanel("DetailInspector", s_detailInspector);

        PanelManager.Instance.AddChildPanel("EditorViewport", new EditorViewport());

        // 创建场景编辑器桥接（事件驱动）
        s_bridge = new SceneEditorBridge(sceneManager);

        // 连接事件总线和缓存引用
        ConnectPanels();
    }

    /// <summary>设置场景浏览器关联的场景句柄。</summary>
    public static void SetSceneHandle(ulong sceneHandle)
    {
        if (s_sceneBrowser != null)
            s_sceneBrowser.SceneHandle = sceneHandle;
        if (s_detailInspector != null)
            s_detailInspector.SceneHandle = sceneHandle;
    }

    /// <summary>连接面板之间的引用（事件总线、层级缓存）。</summary>
    private static void ConnectPanels()
    {
        if (s_sceneBrowser == null || s_detailInspector == null)
            return;

        try
        {
            var eventBus = Neverness.Editor.Core.Public.EditorCoreModule.Context.Events;
            s_sceneBrowser.EventBus = eventBus;
        }
        catch (InvalidOperationException)
        {
            // EditorCoreModule 尚未安装，DetailInspector 会在 OnUpdate 中自行重试
        }

        // 将 SceneBrowser 的缓存引用传递给 DetailInspector
        s_detailInspector.HierarchyCache = s_sceneBrowser.Cache;
    }
}
