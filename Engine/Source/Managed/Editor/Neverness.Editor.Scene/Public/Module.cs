using Neverness.Editor.Core.Public;
using Neverness.Runtime.Scene;
using EditorPlayMode = Neverness.Editor.Core.Public.PlayMode;

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

    /// <summary>设置场景浏览器关联的场景。</summary>
    public static void SetScene(SceneWorld? world)
    {
        Private.SceneModuleImp.SetScene(world);
    }

    // ── PlayMode API ──

    /// <summary>进入播放模式。快照保存失败时返回 false。</summary>
    public static bool EnterPlayMode() => Private.SceneModuleImp.PlayModeController.EnterPlay();

    /// <summary>退出播放模式，恢复快照。</summary>
    public static bool ExitPlayMode() => Private.SceneModuleImp.PlayModeController.ExitPlay();

    /// <summary>暂停——Always 系统继续 tick，Gameplay/Editor 暂停。</summary>
    public static bool Pause() => Private.SceneModuleImp.PlayModeController.Pause();

    /// <summary>恢复——从暂停恢复到播放。</summary>
    public static bool Resume() => Private.SceneModuleImp.PlayModeController.Resume();

    /// <summary>单步执行（Phase 2 预留）。</summary>
    public static bool StepFrame() => Private.SceneModuleImp.PlayModeController.StepFrame();

    /// <summary>当前播放模式。</summary>
    public static EditorPlayMode CurrentPlayMode => Private.SceneModuleImp.PlayModeController.CurrentMode;

    /// <summary>是否在播放。</summary>
    public static bool IsPlaying => Private.SceneModuleImp.PlayModeController.IsPlaying;

    /// <summary>获取当前活动的 SceneWorld（可能为 null）。</summary>
    public static SceneWorld? GetActiveWorld() => Private.SceneModuleImp.PlayModeController.GetActiveWorld();
}
