namespace Neverness.Editor.Assets;

/// <summary>
/// 视频导入设置。
///
/// 存储在 .meta YAML 的 importSettings 字段中。
/// 通过 AssetImportContext.GetSetting*() 读取。
/// </summary>
public sealed class VideoImportSettings
{
    /// <summary>流式加载（默认 true，时长 &lt; 10s 时自动 false）。</summary>
    public bool Streaming { get; set; } = true;

    /// <summary>最大宽度分辨率：0=保持原始 | 854 | 1280 | 1920（默认 1920）。</summary>
    public int MaxResolution { get; set; } = 1920;

    /// <summary>目标码率 kbps：0=保持原始（默认 0）。</summary>
    public int TargetBitrate { get; set; }

    /// <summary>包含音轨（默认 true）。</summary>
    public bool IncludeAudioTrack { get; set; } = true;

    /// <summary>循环播放（默认 false）。</summary>
    public bool LoopPlayback { get; set; }

    /// <summary>预加载完整视频（默认 false，短视频自动 true）。</summary>
    public bool PreloadFullVideo { get; set; }

    /// <summary>提取字幕（预留，默认 false）。</summary>
    public bool ExtractSubtitles { get; set; }

    public static VideoImportSettings Default => new();

    /// <summary>从 AssetImportContext 的字典设置中加载。</summary>
    public static VideoImportSettings FromContext(AssetImportContext context)
    {
        return new VideoImportSettings
        {
            Streaming = context.GetSettingBool("streaming", true),
            MaxResolution = context.GetSettingInt("maxResolution", 1920),
            TargetBitrate = context.GetSettingInt("targetBitrate", 0),
            IncludeAudioTrack = context.GetSettingBool("includeAudioTrack", true),
            LoopPlayback = context.GetSettingBool("loopPlayback", false),
            PreloadFullVideo = context.GetSettingBool("preloadFullVideo", false),
            ExtractSubtitles = context.GetSettingBool("extractSubtitles", false),
        };
    }

    /// <summary>导出为字典（写入 .meta YAML）。</summary>
    public Dictionary<string, string> ToDictionary() => new()
    {
        ["streaming"] = Streaming.ToString().ToLowerInvariant(),
        ["maxResolution"] = MaxResolution.ToString(),
        ["targetBitrate"] = TargetBitrate.ToString(),
        ["includeAudioTrack"] = IncludeAudioTrack.ToString().ToLowerInvariant(),
        ["loopPlayback"] = LoopPlayback.ToString().ToLowerInvariant(),
        ["preloadFullVideo"] = PreloadFullVideo.ToString().ToLowerInvariant(),
        ["extractSubtitles"] = ExtractSubtitles.ToString().ToLowerInvariant(),
    };

    public bool Validate(out string? errorMessage)
    {
        errorMessage = null;

        if (MaxResolution != 0 && MaxResolution != 854 && MaxResolution != 1280 && MaxResolution != 1920)
        {
            errorMessage = $"maxResolution 无效: {MaxResolution}，有效值: 0, 854, 1280, 1920";
            return false;
        }

        if (TargetBitrate < 0)
        {
            errorMessage = $"targetBitrate 不能为负: {TargetBitrate}";
            return false;
        }

        return true;
    }
}
