using System.Numerics;
using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.Core.Public;
using Neverness.Editor.ImGuiEx;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Audio;
using Neverness.Runtime.VFS;
using Neverness.Runtime.VFS;

using ImportPipeline = Neverness.Editor.Assets.ImportPipeline;

namespace Neverness.Editor.Media;

/// <summary>
/// 音频资产打开器——双击音频资产打开波形预览窗口。
///
/// 流程：
///   1. 检查 AssetEditorManager 是否已有该资产的窗口 → 聚焦
///   2. 后台线程：同步加载已导入的资产 + 解析元信息
///   3. 主线程：创建 IAudioPlayer + 通过 IAudioViewerService 打开窗口
/// </summary>
[AssetOpener(AssetTypeId.AudioClip)]
public sealed class AudioAssetOpener : IAssetOpener
{
    private readonly IAudioViewerService _viewerService;
    private readonly AssetEditorManager _editorManager;
    private readonly UIThreadDispatcher _dispatcher;
    private readonly IAudioService _audioService;

    public AudioAssetOpener(
        IAudioViewerService viewerService,
        AssetEditorManager editorManager,
        UIThreadDispatcher dispatcher,
        IAudioService audioService)
    {
        _viewerService = viewerService;
        _editorManager = editorManager;
        _dispatcher = dispatcher;
        _audioService = audioService;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.AudioClip;

    public async Task OpenAsync(AssetOpenContext context)
    {
        // 1. 检查是否已有窗口
        if (_editorManager.TryGetWindowId(context.Guid, out var existingWindowId))
        {
            // TODO: 聚焦窗口
            return;
        }

        // 2. 后台加载元信息
        var (handle, effectiveGuid, metaInfo, vfsPath) = await Task.Run(
            () => LoadAudioAsset(context));

        if (handle.IsZero)
        {
            Console.WriteLine($"[AudioAssetOpener] 加载失败: {context.VirtualPath}");
            return;
        }

        // 3. 主线程：创建播放器 + 通过服务打开窗口
        var tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var path = context.VirtualPath;

        _dispatcher.Enqueue(() =>
        {
            try
            {
                // 创建 Runtime Audio 播放器
                var player = _audioService.CreatePlayer();
                bool loaded = player.Load(vfsPath);
                if (!loaded)
                    Console.WriteLine($"[AudioAssetOpener] 播放器加载失败: {vfsPath}");

                // 通过服务创建查看器窗口
                _viewerService.OpenViewer(new AudioViewerOptions
                {
                    AssetGuidHex = effectiveGuid.ToHexString(),
                    AssetName = path.FileNameWithoutExtension,
                    MetaInfo = metaInfo,
                    Player = loaded ? player : null
                });
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AudioAssetOpener] 异常: {ex}");
            }
            finally
            {
                tcs.TrySetResult();
            }
        });

        await tcs.Task;
    }

    /// <summary>后台加载音频资产并解析元信息。</summary>
    private static (AssetHandle Handle, GUID Guid, AudioMetaInfo Meta, string VfsPath)
        LoadAudioAsset(AssetOpenContext context)
    {
        var handle = AssetHandle.LoadSync(context.Guid, AssetTypeId.AudioClip);
        if (handle.IsZero)
        {
            var importResult = TryImportSource(context);
            if (importResult.Handle.IsZero)
                return (AssetHandle.Zero, context.Guid, default, "");
            handle = importResult.Handle;
        }

        string vfsPath = context.VirtualPath.FullPath;

        string absPath = "";
        try
        {
            var p = VFSService.GetAbsolutePath(vfsPath);
            if (p != null) absPath = p;
        }
        catch { }

        var metaInfo = absPath.Length > 0
            ? ParseAudioMetaInfo(handle, absPath)
            : default;

        return (handle, context.Guid, metaInfo, vfsPath);
    }

    /// <summary>从源文件 + PCM blob 推导音频元信息。</summary>
    private static unsafe AudioMetaInfo ParseAudioMetaInfo(AssetHandle handle, string absPath)
    {
        try
        {
            ulong pcmBlobSize = (ulong)AssetManager.Instance.GetBlobSize(handle.Value, 0);
            if (pcmBlobSize == 0)
                return default;

            if (!File.Exists(absPath))
                return default;

            var ext = Path.GetExtension(absPath).ToLowerInvariant();
            var fileData = File.ReadAllBytes(absPath);

            if (ext == ".wav" && fileData.Length >= 44
                && fileData[0] == 0x52 && fileData[8] == 0x57)
            {
                return ParseWavMeta(fileData, pcmBlobSize);
            }

            if (ext == ".mp3" || (fileData.Length >= 3 && fileData[0] == 0xFF && (fileData[1] & 0xE0) == 0xE0))
            {
                return ParseMp3Meta(fileData, pcmBlobSize);
            }

            ulong sampleCount = pcmBlobSize / 4;
            return new AudioMetaInfo
            {
                SampleRate = 44100, Channels = 2, SampleCount = sampleCount,
                Duration = (double)sampleCount / 44100, Format = ext.TrimStart('.').ToUpperInvariant()
            };
        }
        catch { return default; }
    }

    private static AudioMetaInfo ParseWavMeta(byte[] data, ulong pcmBlobSize)
    {
        var channels = BitConverter.ToUInt16(data, 22);
        var sampleRate = BitConverter.ToUInt32(data, 24);
        var bitsPerSample = BitConverter.ToUInt16(data, 34);
        uint bytesPerSample = (uint)(bitsPerSample / 8) * (uint)channels;
        ulong sampleCount = bytesPerSample > 0 ? pcmBlobSize / bytesPerSample : 0;
        double duration = sampleRate > 0 ? (double)sampleCount / sampleRate : 0;
        return new AudioMetaInfo
        {
            SampleRate = sampleRate, Channels = channels, SampleCount = sampleCount,
            Duration = duration, Format = "WAV"
        };
    }

    private static AudioMetaInfo ParseMp3Meta(byte[] data, ulong blobSize)
    {
        int[,] bitrateTable = {
            {32,32,32,8,8,8},{48,48,40,16,16,16},{56,56,48,24,24,24},{64,64,56,32,32,32},
            {80,80,64,40,40,40},{96,96,80,48,48,48},{112,112,96,56,56,56},{128,128,112,64,64,64},
            {144,144,128,80,80,80},{160,160,144,96,96,96},{176,176,160,112,112,112},{192,192,176,128,128,128},
            {224,224,192,144,144,144},{256,256,224,160,160,160},{320,320,256,176,176,176},{0,0,0,0,0,0}
        };
        int[] sampleRates = { 44100, 48000, 32000 };
        int offset = 0;
        if (data.Length >= 10 && data[0] == 'I' && data[1] == 'D' && data[2] == '3')
        {
            int tagSize = (data[6] << 21) | (data[7] << 14) | (data[8] << 7) | data[9];
            offset = 10 + tagSize;
        }
        uint bitrate = 0; uint sampleRate = 44100; uint channels = 2;
        for (int i = offset; i < data.Length - 4; i++)
        {
            if (data[i] != 0xFF || (data[i + 1] & 0xE0) != 0xE0) continue;
            byte b1 = data[i + 1], b2 = data[i + 2];
            int versionBits = (b1 >> 3) & 0x03;
            int layerBits = (b1 >> 1) & 0x03;
            int bitrateIndex = (b2 >> 4) & 0x0F;
            int srIndex = (b2 >> 2) & 0x03;
            int channelMode = (data[i + 3] >> 6) & 0x03;
            if (bitrateIndex == 0 || bitrateIndex > 14 || srIndex == 3) continue;
            bool isV1 = versionBits == 3;
            int layer = 3 - layerBits;
            int col = isV1 ? layer : 3 + layer;
            if (col > 5) col = 5;
            bitrate = (uint)bitrateTable[bitrateIndex, col] * 1000;
            sampleRate = (uint)sampleRates[srIndex];
            if (!isV1) sampleRate /= 2;
            channels = channelMode == 3 ? 1u : 2u;
            break;
        }
        double duration = bitrate > 0 ? (double)blobSize * 8 / bitrate : 0;
        ulong sampleCount = sampleRate > 0 ? (ulong)(duration * sampleRate) : 0;
        return new AudioMetaInfo
        {
            SampleRate = sampleRate, Channels = channels, SampleCount = sampleCount,
            Duration = duration, Format = "MP3"
        };
    }

    private static (AssetHandle Handle, GUID Guid) TryImportSource(AssetOpenContext context)
    {
        try
        {
            var absolutePath = VFSService.GetAbsolutePath(context.VirtualPath.FullPath);
            if (absolutePath == null) return (AssetHandle.Zero, context.Guid);
            var sourcePath = new NPath(absolutePath);
            if (!File.Exists(sourcePath.FullPath)) return (AssetHandle.Zero, context.Guid);
            Console.WriteLine($"[AudioAssetOpener] 导入: {sourcePath}");
            var result = ImportPipeline.Import(sourcePath);
            if (!result.Success) return (AssetHandle.Zero, context.Guid);
            var effectiveGuid = result.AssetGuid.IsZero ? context.Guid : result.AssetGuid;
            var handle = AssetHandle.LoadSync(effectiveGuid, AssetTypeId.AudioClip);
            return (handle, effectiveGuid);
        }
        catch { return (AssetHandle.Zero, context.Guid); }
    }
}
