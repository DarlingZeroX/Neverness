using Neverness.Editor.Core.Public;
using Neverness.Editor.Scene.Private.Panel;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// 场景编辑器桥接——监听 Runtime SceneManager 事件，驱动 Editor SceneBrowser 刷新。
///
/// 数据流：
///   SceneManager.SceneActivated → SceneModuleImp.SetSceneHandle → SceneBrowser 自动刷新
///   SceneManager.SceneUnloaded → SceneBrowser handle 清零
///
/// 设计原则：
///   - Editor 不持有 Runtime ECS 指针
///   - 事件驱动，非轮询
///   - Snapshot-driven hierarchy（SceneBrowser 内部通过 SceneHierarchyCache 版本轮询实现）
/// </summary>
internal sealed class SceneEditorBridge : IDisposable
{
    private readonly SceneManager _sceneManager;
    private bool _disposed;

    public SceneEditorBridge(SceneManager sceneManager)
    {
        _sceneManager = sceneManager;
        _sceneManager.SceneActivated += OnSceneActivated;
        _sceneManager.SceneUnloaded += OnSceneUnloaded;
    }

    private void OnSceneActivated(IScene scene)
    {
        Console.WriteLine($"[SceneEditorBridge] 场景已激活: {scene.Name}");
        SceneModuleImp.SetScene(scene as SceneWorld);

        // 同步 EditorState：记录当前场景路径
        var state = EditorCoreModule.Context.State;
        if (scene is SceneWorld world)
        {
            state.CurrentScenePath = world.AssetPath;
        }
    }

    private void OnSceneUnloaded(IScene scene)
    {
        Console.WriteLine($"[SceneEditorBridge] 场景已卸载: {scene.Name}");

        // 如果卸载的是当前显示的场景，清零 SceneBrowser handle
        // SceneManager 卸载后可能自动激活下一个场景（会再次触发 SceneActivated）
        // 此处暂不主动清零，让 SceneActivated 事件接管
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        _sceneManager.SceneActivated -= OnSceneActivated;
        _sceneManager.SceneUnloaded -= OnSceneUnloaded;
    }
}
