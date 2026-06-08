namespace Neverness.Editor.Media;

/// <summary>视频元信息（从 NNVideoTypeInfo 解析）。</summary>
public struct VideoMetaInfo
{
    public uint Width;
    public uint Height;
    public uint FpsNum;
    public uint FpsDen;
    public ulong FrameCount;
    public double Duration;
    public string CodecName;
    public bool HasAudio;
    public uint AudioSampleRate;
    public uint AudioChannels;

    public float FPS => FpsDen > 0 ? (float)FpsNum / FpsDen : 0f;
}

/// <summary>音频元信息（从源文件解析）。</summary>
public struct AudioMetaInfo
{
    public uint SampleRate;
    public uint Channels;
    public ulong SampleCount;
    public double Duration;
    public string Format;
}
