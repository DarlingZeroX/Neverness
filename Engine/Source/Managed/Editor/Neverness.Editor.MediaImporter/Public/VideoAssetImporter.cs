using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 视频导入器。
///
/// 支持格式：.mp4, .webm, .mov, .mkv, .avi
///
/// 导入设置（.meta importSettings）：
///   streaming           — true | false（默认 true，时长 < 10s 自动 false）
///   maxResolution       — 0=保持原始 | 1920 | 1280 | 854（默认 1920）
///   includeAudioTrack   — true | false（默认 true）
///   loopPlayback        — true | false（默认 false）
///   preloadFullVideo    — true | false（默认 false，短视频自动 true）
///
/// 注意：视频解码需要通过 C++ Native API（FFmpeg），当前为骨架实现。
/// TODO: 集成 MediaImportBridge P/Invoke 调用 NNRuntimeMediaAssets DLL。
/// </summary>
[AssetImporter(".mp4", ".webm", ".mov", ".mkv", ".avi", Priority = 100)]
public class VideoAssetImporter : ISettingsAwareImporter
{
    public string[] SupportedExtensions => new[] { ".mp4", ".webm", ".mov", ".mkv", ".avi" };
    public string DisplayName => "Video Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.VideoClip);
            var sourceData = context.ReadAllBytes();

            // 读取设置
            var streaming = context.GetSettingBool("streaming", true);
            var maxResolution = context.GetSettingInt("maxResolution", 1920);
            var includeAudio = context.GetSettingBool("includeAudioTrack", true);
            var loopPlayback = context.GetSettingBool("loopPlayback", false);

            // 优先通过 C++ FFmpeg 探测视频元信息，降级为占位值
            var nativeProbe = MediaImportBridge.ProbeFile(context.SourceAssetPath.FullPath);
            var probe = nativeProbe.HasValue
                ? FromNativeProbe(nativeProbe.Value)
                : ProbeVideoPlaceholder(context.Extension, sourceData);

            // 写入 NNVideoTypeInfo (TYPE_INFO blob, 48 字节)
            // C++ 布局：width:u32, height:u32, fpsNum:u32, fpsDen:u32,
            //           frameCount:u64, duration:double, codecId:u32, flags:u32,
            //           audioSampleRate:u32, audioChannels:u32
            uint flags = 0;
            if (probe.HasAudio) flags |= NNVideoFlagValues.HasAudio;
            if (streaming) flags |= NNVideoFlagValues.Streaming;
            if (loopPlayback) flags |= NNVideoFlagValues.Loop;

            var typeInfo = new NNVideoTypeInfoCSharp
            {
                Width = probe.Width,
                Height = probe.Height,
                FpsNum = probe.FpsNum,
                FpsDen = probe.FpsDen,
                FrameCount = probe.FrameCount,
                Duration = probe.Duration,
                CodecId = probe.CodecId,
                Flags = flags,
                AudioSampleRate = probe.HasAudio ? 44100u : 0u,
                AudioChannels = probe.HasAudio ? 2u : 0u
            };
            result.TypeInfo = typeInfo.ToBytes();

            if (streaming)
            {
                // 流式模式：将原始编码数据作为 DATA blob 内嵌
                // Runtime 通过 FFmpeg 解码
                result.Blobs.Add(new ImportedBlob
                {
                    BlobType = AssetTypeId.BlobType.Data,
                    Data = sourceData
                });
            }
            else
            {
                // 非流式模式：TODO 调用 C++ 全量解码
                // 当前降级：存储原始数据
                result.Blobs.Add(new ImportedBlob
                {
                    BlobType = AssetTypeId.BlobType.Data,
                    Data = sourceData
                });
            }

            // 缩略图：TODO 调用 C++ MediaImportBridge.DecodeThumbnail()
            // 当前不生成缩略图

            Console.WriteLine($"[VideoAssetImporter] {context.SourceAssetPath.FileName}: " +
                $"{probe.Width}x{probe.Height} duration={probe.Duration:F1}s streaming={streaming}");

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"视频导入异常: {ex.Message}");
        }
    }

    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["streaming"] = "true",
        ["maxResolution"] = "1920",
        ["includeAudioTrack"] = "true",
        ["loopPlayback"] = "false",
        ["preloadFullVideo"] = "false"
    };

    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;

        if (settings.TryGetValue("maxResolution", out var res) && int.TryParse(res, out var resVal))
        {
            if (resVal != 0 && resVal != 1920 && resVal != 1280 && resVal != 854)
            {
                errorMessage = $"maxResolution 无效: {res}，有效值: 0, 1920, 1280, 854";
                return false;
            }
        }

        return true;
    }

    /* ========== 占位探测（TODO: 替换为 C++ P/Invoke） ========== */

    private struct VideoProbeResult
    {
        public uint Width;
        public uint Height;
        public uint FpsNum;
        public uint FpsDen;
        public ulong FrameCount;
        public double Duration;
        public uint CodecId;
        public bool HasAudio;
    }

    private static VideoProbeResult FromNativeProbe(NNMediaProbeResult p)
    {
        return new VideoProbeResult
        {
            Width = p.Width,
            Height = p.Height,
            FpsNum = p.FpsNum,
            FpsDen = p.FpsDen,
            FrameCount = p.FrameCount,
            Duration = p.Duration,
            CodecId = 0, // TODO: 映射 codecName → NNVideoCodec 枚举
            HasAudio = p.HasAudio != 0
        };
    }

    private static VideoProbeResult ProbeVideoPlaceholder(string extension, byte[] data)
    {
        // 占位实现：返回基本估计值
        // TODO: 替换为 MediaImportBridge.ProbeFile()
        return new VideoProbeResult
        {
            Width = 1920,
            Height = 1080,
            FpsNum = 30000,
            FpsDen = 1001,
            FrameCount = 0, // 未知
            Duration = 0,   // 未知
            CodecId = 0,    // H264
            HasAudio = true
        };
    }
}

/// <summary>
/// C# 侧视频 TypeInfo（镜像 C++ NNVideoTypeInfo，48 字节，8 字节对齐）。
/// C++ 布局：width:u32, height:u32, fpsNum:u32, fpsDen:u32,
///           frameCount:u64, duration:double, codecId:u32, flags:u32,
///           audioSampleRate:u32, audioChannels:u32
/// </summary>
internal struct NNVideoTypeInfoCSharp
{
    public uint Width;            // 4B — 偏移 0
    public uint Height;           // 4B — 偏移 4
    public uint FpsNum;           // 4B — 偏移 8
    public uint FpsDen;           // 4B — 偏移 12
    public ulong FrameCount;      // 8B — 偏移 16
    public double Duration;       // 8B — 偏移 24
    public uint CodecId;          // 4B — 偏移 32
    public uint Flags;            // 4B — 偏移 36
    public uint AudioSampleRate;  // 4B — 偏移 40
    public uint AudioChannels;    // 4B — 偏移 44

    public byte[] ToBytes()
    {
        var buf = new byte[48];
        BitConverter.GetBytes(Width).CopyTo(buf, 0);
        BitConverter.GetBytes(Height).CopyTo(buf, 4);
        BitConverter.GetBytes(FpsNum).CopyTo(buf, 8);
        BitConverter.GetBytes(FpsDen).CopyTo(buf, 12);
        BitConverter.GetBytes(FrameCount).CopyTo(buf, 16);
        BitConverter.GetBytes(Duration).CopyTo(buf, 24);
        BitConverter.GetBytes(CodecId).CopyTo(buf, 32);
        BitConverter.GetBytes(Flags).CopyTo(buf, 36);
        BitConverter.GetBytes(AudioSampleRate).CopyTo(buf, 40);
        BitConverter.GetBytes(AudioChannels).CopyTo(buf, 44);
        return buf;
    }
}

/// <summary>
/// 视频标志位常量（与 C++ NN_VIDEO_FLAG_* 一致）。
/// </summary>
internal static class NNVideoFlagValues
{
    public const uint HasAudio = 1u << 0;
    public const uint Streaming = 1u << 1;
    public const uint HasSubtitles = 1u << 2;
    public const uint AlphaChannel = 1u << 3;
    public const uint Loop = 1u << 4;
}
