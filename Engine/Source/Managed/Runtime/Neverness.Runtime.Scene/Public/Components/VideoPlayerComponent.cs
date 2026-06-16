using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 视频播放器标志位（与 Native NNVideoPlayerFlags 对齐）。
/// </summary>
[Flags]
public enum VideoPlayerFlags : uint
{
    None = 0,
    Playing = 1 << 0,
    Loop = 1 << 1,
    Muted = 1 << 2,
    AutoPlay = 1 << 3,
}

/// <summary>
/// 视频播放器组件——视频播放控制。
/// 替代 C++ NNVideoPlayerComponent（56 字节）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct VideoPlayerComponent : IComponent
{
    /// <summary>视频资产 GUID。</summary>
    public NNGuid VideoClipAsset;

    /// <summary>运行时播放器 ID（瞬态，不序列化）。</summary>
    public uint RuntimePlayerId;

    /// <summary>视频纹理 ID（瞬态，不序列化）。</summary>
    public uint VideoTextureId;

    /// <summary>音量（0.0 ~ 1.0）。</summary>
    public float Volume;

    /// <summary>标志位。</summary>
    public VideoPlayerFlags Flags;

    /// <summary>目标精灵 GUID（视频渲染到此精灵的纹理上）。</summary>
    public NNGuid TargetSprite;

    private uint _reserved0;
    private uint _reserved1;

    /// <summary>创建默认视频播放器组件。</summary>
    public static VideoPlayerComponent Default => new()
    {
        Volume = 1f,
    };
}
