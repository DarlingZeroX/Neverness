namespace Neverness.Editor.Media;

/// <summary>
/// 视频查看器服务接口——由 ImGuiFrontend 模块实现。
/// Media 模块的 AssetOpener 通过此接口创建查看器窗口。
/// </summary>
public interface IVideoViewerService
{
    /// <summary>打开视频查看器窗口。</summary>
    void OpenViewer(VideoViewerOptions options);
}

/// <summary>
/// 视频查看器选项。
/// </summary>
public class VideoViewerOptions
{
    public string? AssetName { get; set; }
    public string? AssetGuidHex { get; set; }
    public VideoMetaInfo MetaInfo { get; set; }
    public ulong ThumbnailHandle { get; set; }
    public System.Numerics.Vector2 ThumbnailSize { get; set; }
}
