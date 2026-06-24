namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器播放模式状态。
/// </summary>
public enum PlayMode
{
    Editing,
    Playing,
    Paused,
}

/// <summary>
/// 编辑器全局状态。
/// </summary>
public sealed class EditorState
{
    /// <summary>当前播放模式。</summary>
    public PlayMode PlayMode { get; set; } = PlayMode.Editing;

    /// <summary>是否处于播放模式。</summary>
    public bool IsPlaying => PlayMode == PlayMode.Playing;

    /// <summary>当前场景 VFSService 路径（null = 无场景）。</summary>
    public string? CurrentScenePath { get; set; }

    /// <summary>当前激活场景的 Native 句柄（0 = 无场景）。</summary>
    public ulong CurrentSceneHandle { get; set; }

    /// <summary>当前选中实体句柄值（0 = 无选中）。</summary>
    public ulong SelectedEntityHandle { get; set; }
}
