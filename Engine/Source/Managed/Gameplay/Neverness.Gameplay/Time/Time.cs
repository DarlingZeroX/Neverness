// ============================================================================
// Time.cs - 时间系统静态 API
// ============================================================================
// 时间系统静态 API，通过 GameplayContext 获取 ITimeProvider。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 时间系统静态 API。
/// </summary>
/// <remarks>
/// 通过 GameplayContext 获取 ITimeProvider，避免 static 全局污染。
/// </remarks>
public static class Time
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>时间提供者。</summary>
    private static ITimeProvider? _provider;

    // ========================================================================
    // 初始化
    // ========================================================================

    /// <summary>设置时间提供者（由引擎初始化时调用）。</summary>
    /// <param name="provider">时间提供者。</param>
    internal static void SetProvider(ITimeProvider provider)
    {
        _provider = provider;
    }

    // ========================================================================
    // 属性
    // ========================================================================

    /// <summary>帧间隔时间（秒）。</summary>
    public static float DeltaTime => _provider?.DeltaTime ?? 0f;

    /// <summary>固定时间步（秒）。</summary>
    public static float FixedDeltaTime => _provider?.FixedDeltaTime ?? 0f;

    /// <summary>游戏运行时间（秒）。</summary>
    public static float TimeSinceStartup => _provider?.TimeSinceStartup ?? 0f;

    /// <summary>帧计数。</summary>
    public static ulong FrameCount => _provider?.FrameCount ?? 0;

    /// <summary>时间缩放。</summary>
    public static float TimeScale
    {
        get => _provider?.TimeScale ?? 1f;
        set
        {
            if (_provider != null)
            {
                _provider.TimeScale = value;
            }
        }
    }
}
