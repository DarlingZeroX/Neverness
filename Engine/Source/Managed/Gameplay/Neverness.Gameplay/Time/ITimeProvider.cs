// ============================================================================
// ITimeProvider.cs - 时间提供者接口
// ============================================================================
// 时间提供者接口，避免 static 全局污染，支持确定性模拟。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 时间提供者接口：避免 static 全局污染，支持确定性模拟。
/// </summary>
public interface ITimeProvider
{
    /// <summary>帧间隔时间（秒）。</summary>
    float DeltaTime { get; }

    /// <summary>固定时间步（秒）。</summary>
    float FixedDeltaTime { get; }

    /// <summary>游戏运行时间（秒）。</summary>
    float TimeSinceStartup { get; }

    /// <summary>帧计数。</summary>
    ulong FrameCount { get; }

    /// <summary>时间缩放。</summary>
    float TimeScale { get; set; }
}
