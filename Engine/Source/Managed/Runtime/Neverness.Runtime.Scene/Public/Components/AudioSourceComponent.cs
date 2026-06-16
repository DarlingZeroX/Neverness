using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 音频源标志位（与 Native NNAudioSourceFlags 对齐）。
/// </summary>
[Flags]
public enum AudioSourceFlags : uint
{
    None = 0,
    Playing = 1 << 0,
    Loop = 1 << 1,
    Spatial = 1 << 2,
    AutoPlay = 1 << 3,
}

/// <summary>
/// 音频源组件——音频播放控制。
/// 替代 C++ NNAudioSourceComponent（48 字节）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct AudioSourceComponent : IComponent
{
    /// <summary>音频资产 GUID。</summary>
    public NNGuid AudioClipAsset;

    /// <summary>运行时播放器 ID（瞬态，不序列化）。</summary>
    public uint RuntimePlayerId;

    /// <summary>音量（0.0 ~ 1.0）。</summary>
    public float Volume;

    /// <summary>音调（1.0 = 原速）。</summary>
    public float Pitch;

    /// <summary>最小距离（空间音频）。</summary>
    public float MinDistance;

    /// <summary>最大距离（空间音频）。</summary>
    public float MaxDistance;

    /// <summary>标志位。</summary>
    public AudioSourceFlags Flags;

    private uint _reserved0;
    private uint _reserved1;

    /// <summary>创建默认音频源组件。</summary>
    public static AudioSourceComponent Default => new()
    {
        Volume = 1f,
        Pitch = 1f,
        MinDistance = 1f,
        MaxDistance = 100f,
    };
}
