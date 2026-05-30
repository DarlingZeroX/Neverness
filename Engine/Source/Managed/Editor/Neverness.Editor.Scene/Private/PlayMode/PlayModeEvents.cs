using EditorPlayMode = Neverness.Editor.Core.Public.PlayMode;

namespace Neverness.Editor.Scene.Private.PlayMode;

/// <summary>
/// 播放模式生命周期事件类型。
/// 细粒度事件，供插件（Profiler、Audio Preview、Animation Preview）挂载。
/// </summary>
public enum PlayModeEventType
{
    /// <summary>即将进入播放模式（快照已保存，尚未开始 tick）。</summary>
    EnteringPlayMode,

    /// <summary>已进入播放模式（Gameplay 系统开始 tick）。</summary>
    EnteredPlayMode,

    /// <summary>即将退出播放模式（即将恢复快照）。</summary>
    ExitingPlayMode,

    /// <summary>已退出播放模式（快照已恢复，回到编辑态）。</summary>
    ExitedPlayMode,

    /// <summary>已暂停（Always 系统继续，Gameplay/Editor 暂停）。</summary>
    Paused,

    /// <summary>已恢复（从暂停恢复到播放）。</summary>
    Resumed,
}

/// <summary>播放模式生命周期事件数据。</summary>
public readonly record struct PlayModeEvent(PlayModeEventType Type);

/// <summary>
/// PlayModeChanged 事件 payload——类型化，避免字符串拼写错误。
/// 通过 EditorEventBus 分发，挂在 EditorEventType.PlayModeChanged 上。
/// </summary>
public readonly record struct PlayModeChangedEvent(
    EditorPlayMode OldMode,
    EditorPlayMode NewMode);
