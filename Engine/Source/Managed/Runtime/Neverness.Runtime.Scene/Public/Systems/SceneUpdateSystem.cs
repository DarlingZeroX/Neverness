namespace Neverness.Runtime.Scene.Systems;

/// <summary>
/// 场景更新系统——全局场景逻辑钩子。
/// 替代 C++ NNSceneUpdateSystem。
/// </summary>
public sealed class SceneUpdateSystem : ISceneSystem
{
    private IScene? _scene;
    private ulong _frameCount;

    public string Name => "SceneUpdateSystem";
    public int Priority => 50; // 在 HierarchySystem 之后，TransformSystem 之前
    public SceneSystemTags Tags => SceneSystemTags.All;
    public bool IsInitialized { get; private set; }

    public void Initialize(IScene scene)
    {
        _scene = scene;
        _frameCount = 0;
        IsInitialized = true;
    }

    public void Update(float deltaTime)
    {
        if (_scene == null) return;

        _frameCount++;

        // TODO: 全局场景逻辑
        // - 场景状态更新
        // - 全局事件处理
        // - 预加载/卸载逻辑
    }

    public void FixedUpdate(float fixedDeltaTime) { }

    public void Shutdown()
    {
        _scene = null;
        IsInitialized = false;
    }

    public void Dispose() { }

    // ── 扩展 API ──

    /// <summary>当前帧计数。</summary>
    public ulong FrameCount => _frameCount;
}
