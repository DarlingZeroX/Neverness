using Neverness.Runtime.Engine;

namespace Neverness.Editor.Scene.Private.PlayMode;

/// <summary>
/// 播放模式前快照——保存编辑态状态，退出播放时恢复。
/// 由 PlayModeController 内部持有，不污染 EditorState。
/// </summary>
internal sealed class PlayModeSnapshot
{
    /// <summary>
    /// 快照 VFSService 路径。
    /// 格式：Cache/PlayModeSnapshots/{guid}.vgsc
    /// 使用 Guid.NewGuid() 生成，避免多次 Play/Stop 或多实例冲突。
    /// </summary>
    public string SnapshotPath { get; init; } = "";

    /// <summary>场景 GUID（用于恢复时索引，不靠名字）。</summary>
    public NNGuid SceneGuid { get; init; }

    /// <summary>场景资产路径（null = 未保存的新场景）。</summary>
    public string? SceneAssetPath { get; init; }

    /// <summary>进入播放前选中的实体句柄（0 = 无选中）。</summary>
    public ulong SelectedEntityHandle { get; init; }

    /// <summary>捕获时间戳。</summary>
    public DateTimeOffset CapturedAt { get; init; }
}
