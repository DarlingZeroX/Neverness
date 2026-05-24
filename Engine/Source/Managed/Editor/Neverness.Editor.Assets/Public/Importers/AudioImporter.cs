using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 音频导入器。
///
/// 支持格式：.wav, .ogg, .mp3, .flac
///
/// 导入设置：
///   loadType     — DecompressOnLoad | CompressedInMemory | Streaming（默认 CompressedInMemory）
///   compression  — PCM | Vorbis | Opus（默认 Vorbis）
///   quality      — 0.0 ~ 1.0（默认 0.7）
///   sampleRate   — 22050 | 44100 | 48000（默认 44100）
/// </summary>
[AssetImporter(".wav", ".ogg", ".mp3", ".flac")]
public class AudioImporter : ISettingsAwareImporter
{
    public string[] SupportedExtensions => new[] { ".wav", ".ogg", ".mp3", ".flac" };
    public string DisplayName => "Audio Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.AudioClip);

            var sourceData = context.ReadAllBytes();

            /* TODO: 实际应使用 NAudio/Vorbis 解码为 PCM */
            var audioData = DecodeAudio(context.Extension, sourceData);

            if (audioData.PcmData == null || audioData.PcmData.Length == 0)
                return ImportResult.Fail($"无法解码音频: {context.SourceAssetPath}");

            /* 写入 PCM 数据 blob */
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.AudioPcm,
                Data = audioData.PcmData
            });

            /* 写入 seek table（用于 streaming） */
            if (audioData.SeekTable != null && audioData.SeekTable.Length > 0)
            {
                result.Blobs.Add(new ImportedBlob
                {
                    BlobType = AssetTypeId.BlobType.AudioSeek,
                    Data = audioData.SeekTable
                });
            }

            /* 写入 TypeInfo */
            var typeInfo = new NNAudioTypeInfoCSharp
            {
                SampleRate = (uint)audioData.SampleRate,
                Channels = (uint)audioData.Channels,
                SampleCount = (uint)audioData.SampleCount,
                DurationMs = (uint)(audioData.DurationSeconds * 1000)
            };
            result.TypeInfo = typeInfo.ToBytes();

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"音频导入异常: {ex.Message}");
        }
    }

    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["loadType"] = "CompressedInMemory",
        ["compression"] = "Vorbis",
        ["quality"] = "0.7",
        ["sampleRate"] = "44100"
    };

    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;

        if (settings.TryGetValue("quality", out var q))
        {
            if (float.TryParse(q, out var quality) && (quality < 0f || quality > 1f))
            {
                errorMessage = "quality 必须在 0.0 ~ 1.0 之间";
                return false;
            }
        }

        return true;
    }

    /* ========== 内部数据结构 ========== */

    private class DecodedAudio
    {
        public byte[] PcmData = Array.Empty<byte>();
        public byte[]? SeekTable;
        public int SampleRate = 44100;
        public int Channels = 2;
        public long SampleCount;
        public double DurationSeconds;
    }

    private DecodedAudio DecodeAudio(string extension, byte[] raw)
    {
        /* WAV 可以简单解析头部 */
        if (extension == ".wav" && raw.Length > 44)
        {
            /* 检查 RIFF magic */
            if (raw[0] == 0x52 && raw[1] == 0x49 && raw[2] == 0x46 && raw[3] == 0x46)
            {
                var channels = BitConverter.ToUInt16(raw, 22);
                var sampleRate = BitConverter.ToInt32(raw, 24);
                var bitsPerSample = BitConverter.ToUInt16(raw, 34);

                /* 查找 data chunk */
                int dataOffset = 44;
                int dataSize = raw.Length - dataOffset;
                var pcmData = new byte[dataSize];
                Array.Copy(raw, dataOffset, pcmData, 0, dataSize);

                var bytesPerSample = bitsPerSample / 8 * channels;
                var sampleCount = bytesPerSample > 0 ? dataSize / bytesPerSample : 0;
                var duration = sampleRate > 0 ? (double)sampleCount / sampleRate : 0;

                return new DecodedAudio
                {
                    PcmData = pcmData,
                    SampleRate = sampleRate,
                    Channels = channels,
                    SampleCount = sampleCount,
                    DurationSeconds = duration
                };
            }
        }

        /* TODO: OGG/MP3/FLAC 解码需要第三方库 */
        return new DecodedAudio
        {
            PcmData = raw, /* 降级：直接传递原始数据 */
            SampleRate = 44100,
            Channels = 2,
            SampleCount = raw.Length / 4,
            DurationSeconds = (raw.Length / 4.0) / 44100.0
        };
    }
}

internal struct NNAudioTypeInfoCSharp
{
    public uint SampleRate;
    public uint Channels;
    public uint SampleCount;
    public uint DurationMs;

    public byte[] ToBytes()
    {
        var buf = new byte[16];
        BitConverter.GetBytes(SampleRate).CopyTo(buf, 0);
        BitConverter.GetBytes(Channels).CopyTo(buf, 4);
        BitConverter.GetBytes(SampleCount).CopyTo(buf, 8);
        BitConverter.GetBytes(DurationMs).CopyTo(buf, 12);
        return buf;
    }
}
