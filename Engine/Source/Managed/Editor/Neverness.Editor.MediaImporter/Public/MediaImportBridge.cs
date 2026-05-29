using System.Runtime.InteropServices;

namespace Neverness.Editor.Assets;

/// <summary>
/// 媒体导入 C++ 桥接层。
///
/// 通过 NativeLibrary.Load() 延迟加载 NNRuntimeMediaAssets DLL，
/// 解析 C ABI 导出函数，供 AudioImporter / VideoImporter 调用。
///
/// 遵循 Neverness Editor 端的 P/Invoke 模式：
/// - 禁止 [DllImport]（与 Runtime.Interop 层规则一致）
/// - 使用 NativeLibrary.Load + GetFunctionDelegate 延迟加载
/// - DLL 未就绪时所有方法返回 false / null（优雅降级）
/// </summary>
public static unsafe class MediaImportBridge
{
    private const string DllName = "NevernessRuntime-MediaAssets";

    private static nint s_libraryHandle = 0;
    private static bool s_loadAttempted;
    private static string? s_loadError;

    // ── 函数指针委托（与 NNMediaCooker.h C ABI 一致） ──

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int DelProbeFile(byte* filePath, NNMediaProbeResult* outResult);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int DelExtractPCM(
        byte* filePath, uint targetChannels, uint targetSampleRate,
        byte** outData, ulong* outSize);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int DelDecodeThumbnail(
        byte* filePath, uint maxWidth,
        byte** outRGBA, uint* outWidth, uint* outHeight);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate int DelBuildSeekTable(
        byte* filePath, byte** outData, ulong* outSize);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void DelFreeBuffer(byte* buffer);

    // ── 解析后的函数指针 ──

    private static DelProbeFile? s_probeFile;
    private static DelExtractPCM? s_extractPCM;
    private static DelDecodeThumbnail? s_decodeThumbnail;
    private static DelBuildSeekTable? s_buildAudioSeekTable;
    private static DelBuildSeekTable? s_buildVideoSeekTable;
    private static DelFreeBuffer? s_freeBuffer;

    // ── 公共 API ──

    /// <summary>探测媒体文件元信息。DLL 未就绪时返回 null。</summary>
    public static NNMediaProbeResult? ProbeFile(string filePath)
    {
        if (!EnsureLoaded()) return null;

        fixed (byte* pPath = System.Text.Encoding.UTF8.GetBytes(filePath + '\0'))
        {
            NNMediaProbeResult result;
            if (s_probeFile!(pPath, &result) == 0)
                return result;
        }
        return null;
    }

    /// <summary>提取 PCM 数据。DLL 未就绪时返回 null。</summary>
    public static byte[]? ExtractPCM(string filePath, uint channels, uint sampleRate)
    {
        if (!EnsureLoaded()) return null;

        fixed (byte* pPath = System.Text.Encoding.UTF8.GetBytes(filePath + '\0'))
        {
            byte* pData = null;
            ulong size = 0;

            if (s_extractPCM!(pPath, channels, sampleRate, &pData, &size) != 0
                || pData == null || size == 0)
                return null;

            try
            {
                var result = new byte[size];
                Marshal.Copy((nint)pData, result, 0, (int)size);
                return result;
            }
            finally
            {
                s_freeBuffer!(pData);
            }
        }
    }

    /// <summary>解码首帧缩略图（RGBA）。DLL 未就绪时返回 null。</summary>
    public static DecodedImage? DecodeThumbnail(string filePath, uint maxWidth = 256)
    {
        if (!EnsureLoaded()) return null;

        fixed (byte* pPath = System.Text.Encoding.UTF8.GetBytes(filePath + '\0'))
        {
            byte* pRGBA = null;
            uint w = 0, h = 0;

            if (s_decodeThumbnail!(pPath, maxWidth, &pRGBA, &w, &h) != 0
                || pRGBA == null || w == 0 || h == 0)
                return null;

            try
            {
                var pixelCount = (int)(w * h * 4);
                var pixels = new byte[pixelCount];
                Marshal.Copy((nint)pRGBA, pixels, 0, pixelCount);
                return new DecodedImage { Width = w, Height = h, Pixels = pixels };
            }
            finally
            {
                s_freeBuffer!(pRGBA);
            }
        }
    }

    /// <summary>构建音频 Seek 表。DLL 未就绪时返回 null。</summary>
    public static byte[]? BuildAudioSeekTable(string filePath)
    {
        if (!EnsureLoaded()) return null;
        return BuildSeekTable(filePath, s_buildAudioSeekTable);
    }

    /// <summary>构建视频 Seek 表。DLL 未就绪时返回 null。</summary>
    public static byte[]? BuildVideoSeekTable(string filePath)
    {
        if (!EnsureLoaded()) return null;
        return BuildSeekTable(filePath, s_buildVideoSeekTable);
    }

    /// <summary>是否已成功加载 DLL。</summary>
    public static bool IsLoaded => s_libraryHandle != 0;

    /// <summary>加载错误信息（如果有）。</summary>
    public static string? LoadError => s_loadError;

    // ── 内部实现 ──

    private static bool EnsureLoaded()
    {
        if (s_libraryHandle != 0) return true;
        if (s_loadAttempted) return false;
        s_loadAttempted = true;

        try
        {
            s_libraryHandle = NativeLibrary.Load(DllName);

            s_probeFile = Resolve<DelProbeFile>(s_libraryHandle, "NNMediaProbeFile");
            s_extractPCM = Resolve<DelExtractPCM>(s_libraryHandle, "NNMediaExtractPCM");
            s_decodeThumbnail = Resolve<DelDecodeThumbnail>(s_libraryHandle, "NNMediaDecodeThumbnail");
            s_buildAudioSeekTable = Resolve<DelBuildSeekTable>(s_libraryHandle, "NNMediaBuildAudioSeekTable");
            s_buildVideoSeekTable = Resolve<DelBuildSeekTable>(s_libraryHandle, "NNMediaBuildVideoSeekTable");
            s_freeBuffer = Resolve<DelFreeBuffer>(s_libraryHandle, "NNMediaFreeBuffer");

            if (s_probeFile == null || s_extractPCM == null || s_freeBuffer == null)
            {
                s_loadError = $"{DllName}: 缺少必要的导出函数";
                return false;
            }

            Console.WriteLine($"[MediaImportBridge] 已加载 {DllName} DLL");
            return true;
        }
        catch (DllNotFoundException ex)
        {
            s_loadError = $"{DllName} DLL 未找到: {ex.Message}";
            return false;
        }
        catch (Exception ex)
        {
            s_loadError = $"加载 {DllName} 失败: {ex.Message}";
            return false;
        }
    }

    private static TDelegate? Resolve<TDelegate>(nint handle, string name)
        where TDelegate : class
    {
        if (NativeLibrary.TryGetExport(handle, name, out var ptr))
            return Marshal.GetDelegateForFunctionPointer<TDelegate>(ptr);
        return null;
    }

    private static byte[]? BuildSeekTable(string filePath, DelBuildSeekTable? fn)
    {
        if (fn == null) return null;

        fixed (byte* pPath = System.Text.Encoding.UTF8.GetBytes(filePath + '\0'))
        {
            byte* pData = null;
            ulong size = 0;

            if (fn(pPath, &pData, &size) != 0 || pData == null || size == 0)
                return null;

            try
            {
                var result = new byte[size];
                Marshal.Copy((nint)pData, result, 0, (int)size);
                return result;
            }
            finally
            {
                s_freeBuffer!(pData);
            }
        }
    }
}

// ── 数据类型 ──

/// <summary>
/// 媒体探测结果（与 C++ NNMediaProbeResult 56 字节布局一致）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct NNMediaProbeResult
{
    public uint MediaType;       // 0=audio, 1=video
    public uint Width;
    public uint Height;
    public uint SampleRate;
    public uint Channels;
    public double Duration;
    public uint FpsNum;
    public uint FpsDen;
    public ulong FrameCount;
    public uint HasAudio;
    public uint HasAlpha;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string CodecName;

    public bool IsAudio => MediaType == 0;
    public bool IsVideo => MediaType == 1;
}

/// <summary>解码后的图像数据（RGBA）。</summary>
public sealed class DecodedImage
{
    public uint Width;
    public uint Height;
    public byte[] Pixels = Array.Empty<byte>();
}
