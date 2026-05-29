namespace Neverness.Editor.Assets;

/// <summary>
/// 音频导入设置。
///
/// 存储在 .meta YAML 的 importSettings 字段中。
/// 通过 AssetImportContext.GetSetting*() 读取。
/// </summary>
public sealed class AudioImportSettings
{
    /// <summary>压缩格式：PCM16 | Float32 | Opus | Vorbis（默认 PCM16）。</summary>
    public string CompressionFormat { get; set; } = "PCM16";

    /// <summary>强制转为单声道（默认 false）。</summary>
    public bool ForceToMono { get; set; }

    /// <summary>归一化音频（默认 false）。</summary>
    public bool NormalizeAudio { get; set; }

    /// <summary>流式加载（默认 false，文件 >= 5MB 时自动 true）。</summary>
    public bool Streaming { get; set; }

    /// <summary>目标采样率：0=保持原始 | 22050 | 44100 | 48000（默认 44100）。</summary>
    public int SampleRate { get; set; } = 44100;

    /// <summary>压缩质量 [0.0, 1.0]，仅 Opus/Vorbis 有效（默认 0.7）。</summary>
    public float Quality { get; set; } = 0.7f;

    public static AudioImportSettings Default => new();

    /// <summary>从 AssetImportContext 的字典设置中加载。</summary>
    public static AudioImportSettings FromContext(AssetImportContext context)
    {
        return new AudioImportSettings
        {
            CompressionFormat = context.GetSetting("compressionFormat", "PCM16"),
            ForceToMono = context.GetSettingBool("forceToMono", false),
            NormalizeAudio = context.GetSettingBool("normalizeAudio", false),
            Streaming = context.GetSettingBool("streaming", false),
            SampleRate = context.GetSettingInt("sampleRate", 44100),
            Quality = context.GetSettingFloat("quality", 0.7f),
        };
    }

    /// <summary>导出为字典（写入 .meta YAML）。</summary>
    public Dictionary<string, string> ToDictionary() => new()
    {
        ["compressionFormat"] = CompressionFormat,
        ["forceToMono"] = ForceToMono.ToString().ToLowerInvariant(),
        ["normalizeAudio"] = NormalizeAudio.ToString().ToLowerInvariant(),
        ["streaming"] = Streaming.ToString().ToLowerInvariant(),
        ["sampleRate"] = SampleRate.ToString(),
        ["quality"] = Quality.ToString("F1"),
    };

    public bool Validate(out string? errorMessage)
    {
        errorMessage = null;

        var validFormats = new[] { "PCM16", "Float32", "Opus", "Vorbis" };
        if (!validFormats.Contains(CompressionFormat, StringComparer.OrdinalIgnoreCase))
        {
            errorMessage = $"不支持的压缩格式: {CompressionFormat}，有效值: {string.Join(", ", validFormats)}";
            return false;
        }

        if (Quality < 0f || Quality > 1f)
        {
            errorMessage = $"quality 必须在 0.0 ~ 1.0 之间: {Quality}";
            return false;
        }

        if (SampleRate != 0 && SampleRate != 22050 && SampleRate != 44100 && SampleRate != 48000)
        {
            errorMessage = $"sampleRate 无效: {SampleRate}，有效值: 0, 22050, 44100, 48000";
            return false;
        }

        return true;
    }
}
