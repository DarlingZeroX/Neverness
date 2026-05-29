namespace Neverness.Runtime.Engine;

/// <summary>
/// 视频播放器组件标志位（与 Native <c>NNVideoPlayerComponent.h</c> 对齐）。
/// </summary>
[Flags]
public enum NNVideoPlayerFlags : uint
{
    None = 0,
    PlayOnAwake = 1 << 0,       // 场景启动时自动播放
    Loop = 1 << 1,              // 循环播放
    RenderToSprite = 1 << 2,    // 渲染到 SpriteRenderer
    Mute = 1 << 3,              // 静音
    Paused = 1 << 4,            // 暂停状态
}
