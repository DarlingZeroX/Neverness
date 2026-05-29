namespace Neverness.Runtime.Engine;

/// <summary>
/// 音频源组件标志位（与 Native <c>NNAudioSourceComponent.h</c> 对齐）。
/// </summary>
[Flags]
public enum NNAudioSourceFlags : uint
{
    None = 0,
    PlayOnAwake = 1 << 0,   // 场景启动时自动播放
    Loop = 1 << 1,          // 循环播放
    Spatial = 1 << 2,       // 启用 3D 空间音频
    Mute = 1 << 3,          // 静音
}
