using Neverness.Editor.Core.Public;
using Neverness.Runtime.Scene;
using EditorPlayMode = Neverness.Editor.Core.Public.PlayMode;

namespace Neverness.Editor.Scene.Private.PlayMode;

/// <summary>
/// 播放模式控制器——Editor 侧核心，驱动 PlayMode 状态机。
/// Runtime.Scene 完全不知道 PlayMode 的存在。
///
/// 职责：
/// - EnterPlay / ExitPlay / Pause / Resume / StepFrame
/// - 根据当前模式选择 tagMask 驱动 SceneWorld tick
/// - 快照保存/恢复（原子操作，异常安全）
/// - 生命周期事件分发
/// </summary>
internal sealed class PlayModeController
{
    private readonly SceneManager _sceneManager;
    private readonly EditorState _editorState;
    private readonly IEditorEventBus _eventBus;

    private PlayModeSnapshot? _prePlaySnapshot;
    private EditorPlayMode _currentMode = EditorPlayMode.Editing;

    /// <summary>播放模式生命周期事件（细粒度，供插件挂载）。</summary>
    public event Action<PlayModeEvent>? PlayModeLifecycle;

    // ── Editor 决定每个模式的 tagMask（组合标签在此定义，不放 Runtime）──

    private static readonly SceneSystemTags s_editingMask =
        SceneSystemTags.Editor |
        SceneSystemTags.Render |
        SceneSystemTags.Audio |
        SceneSystemTags.Streaming;

    private static readonly SceneSystemTags s_playingMask =
        SceneSystemTags.Gameplay |
        SceneSystemTags.Physics |
        SceneSystemTags.Render |
        SceneSystemTags.Audio |
        SceneSystemTags.Streaming;

    private static readonly SceneSystemTags s_pausedMask =
        SceneSystemTags.Render |
        SceneSystemTags.Audio |
        SceneSystemTags.Streaming;

    // ── 公开状态 ──

    public EditorPlayMode CurrentMode => _currentMode;
    public bool IsPlaying => _currentMode == EditorPlayMode.Playing;
    public bool IsPaused => _currentMode == EditorPlayMode.Paused;

    // ── 构造 ──

    public PlayModeController(SceneManager sceneManager, EditorState editorState, IEditorEventBus eventBus)
    {
        _sceneManager = sceneManager;
        _editorState = editorState;
        _eventBus = eventBus;
    }

    // ── 生命周期（异常安全）──

    /// <summary>
    /// 进入播放模式。
    /// 快照保存失败时不会切换状态，保证原子性。
    /// </summary>
    public bool EnterPlay()
    {
        if (_currentMode != EditorPlayMode.Editing) return false;
        if (!_sceneManager.HasActiveScene) return false;

        var oldMode = _currentMode;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.EnteringPlayMode));

        // 快照可能失败（磁盘满、权限、VFS 异常），必须异常安全
        try
        {
            _prePlaySnapshot = CaptureSnapshot();
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[PlayModeController] 快照保存失败: {ex.Message}");
            return false;
        }

        _currentMode = EditorPlayMode.Playing;
        _editorState.PlayMode = EditorPlayMode.Playing;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.EnteredPlayMode));
        _eventBus.Emit(new EditorEvent(EditorEventType.PlayModeChanged,
            new PlayModeChangedEvent(oldMode, EditorPlayMode.Playing)));

        Console.WriteLine("[PlayModeController] 进入播放模式");
        return true;
    }

    /// <summary>
    /// 退出播放模式——恢复快照，回到编辑态。
    /// </summary>
    public bool ExitPlay()
    {
        if (_currentMode != EditorPlayMode.Playing && _currentMode != EditorPlayMode.Paused)
            return false;

        var oldMode = _currentMode;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.ExitingPlayMode));

        RestoreSnapshot();

        _currentMode = EditorPlayMode.Editing;
        _editorState.PlayMode = EditorPlayMode.Editing;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.ExitedPlayMode));
        _eventBus.Emit(new EditorEvent(EditorEventType.PlayModeChanged,
            new PlayModeChangedEvent(oldMode, EditorPlayMode.Editing)));

        CleanupSnapshot();

        Console.WriteLine("[PlayModeController] 退出播放模式");
        return true;
    }

    /// <summary>暂停——Always 系统继续 tick，Gameplay/Editor 暂停。</summary>
    public bool Pause()
    {
        if (_currentMode != EditorPlayMode.Playing) return false;

        var oldMode = _currentMode;

        _currentMode = EditorPlayMode.Paused;
        _editorState.PlayMode = EditorPlayMode.Paused;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.Paused));
        _eventBus.Emit(new EditorEvent(EditorEventType.PlayModeChanged,
            new PlayModeChangedEvent(oldMode, EditorPlayMode.Paused)));

        Console.WriteLine("[PlayModeController] 暂停");
        return true;
    }

    /// <summary>恢复——从暂停恢复到播放。</summary>
    public bool Resume()
    {
        if (_currentMode != EditorPlayMode.Paused) return false;

        var oldMode = _currentMode;

        _currentMode = EditorPlayMode.Playing;
        _editorState.PlayMode = EditorPlayMode.Playing;

        PlayModeLifecycle?.Invoke(new PlayModeEvent(PlayModeEventType.Resumed));
        _eventBus.Emit(new EditorEvent(EditorEventType.PlayModeChanged,
            new PlayModeChangedEvent(oldMode, EditorPlayMode.Playing)));

        Console.WriteLine("[PlayModeController] 恢复播放");
        return true;
    }

    /// <summary>单步执行——暂停状态下执行一帧后继续暂停。Phase 1 预留接口。</summary>
    public bool StepFrame()
    {
        // TODO: Phase 2 实现
        throw new NotImplementedException("Frame Step 将在 Phase 2 实现");
    }

    /// <summary>获取当前活动的 SceneWorld。</summary>
    public SceneWorld? GetActiveWorld() => _sceneManager.ActiveWorld;

    // ── Tick 驱动（由 SceneSubsystem 委托调用）──

    /// <summary>
    /// 每帧调用——根据当前模式选择 tagMask，驱动 SceneWorld tick。
    /// 由 SceneSubsystem.Tick() 委托调用，保持 RuntimeLoop 统一驱动。
    /// </summary>
    public void TickActiveScene(float deltaTime)
    {
        var world = _sceneManager.ActiveWorld;
        if (world == null || !world.IsValid) return;

        var mask = _currentMode switch
        {
            EditorPlayMode.Editing => s_editingMask,
            EditorPlayMode.Playing => s_playingMask,
            EditorPlayMode.Paused => s_pausedMask,
            _ => s_editingMask,
        };

        world.TickByTagMask(deltaTime, mask);
    }

    // ── 快照内部实现 ──

    private PlayModeSnapshot CaptureSnapshot()
    {
        var world = _sceneManager.ActiveWorld!;

        // 用 Guid.NewGuid() 而非 sceneGuid，避免多次 Play/Stop 或多实例冲突
        var snapshotPath = $"/Cache/PlayModeSnapshots/{Guid.NewGuid():N}.vgsc";

        world.Save(snapshotPath);

        return new PlayModeSnapshot
        {
            SnapshotPath = snapshotPath,
            SceneGuid = world.AssetGuid,
            SceneAssetPath = world.AssetPath,
            SelectedEntityHandle = _editorState.SelectedEntityHandle,
            CapturedAt = DateTimeOffset.UtcNow,
        };
    }

    private void RestoreSnapshot()
    {
        if (_prePlaySnapshot == null) return;

        // 原子替换——不触发事件风暴，不影响其他子场景
        _sceneManager.ReplaceActiveSceneFromSnapshot(
            _prePlaySnapshot.SceneGuid,
            _prePlaySnapshot.SnapshotPath);

        _editorState.SelectedEntityHandle = _prePlaySnapshot.SelectedEntityHandle;
    }

    private void CleanupSnapshot()
    {
        if (_prePlaySnapshot == null) return;

        try
        {
            // TODO: VFS.Delete(_prePlaySnapshot.SnapshotPath);
        }
        catch
        {
            // 清理失败不影响主流程
        }

        _prePlaySnapshot = null;
    }
}
