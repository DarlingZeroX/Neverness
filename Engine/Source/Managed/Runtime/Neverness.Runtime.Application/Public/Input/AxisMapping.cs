// Neverness.Runtime.Application — 轴映射定义。
// 定义 GetAxis/GetAxisRaw 的键位映射。

namespace Neverness.Runtime.Application;

/// <summary>
/// 轴映射定义。
/// 将轴名称（如 "Horizontal"）映射到正/负方向键。
/// </summary>
public sealed class AxisMapping
{
    /// <summary>正方向键。</summary>
    public KeyCode PositiveKey { get; init; }

    /// <summary>负方向键。</summary>
    public KeyCode NegativeKey { get; init; }

    /// <summary>平滑灵敏度（越大响应越快）。</summary>
    public float Sensitivity { get; init; } = 3f;

    /// <summary>是否为原始值（无平滑，GetAxis 也返回瞬时值）。</summary>
    public bool Raw { get; init; }
}

/// <summary>
/// 默认轴配置。
/// </summary>
public static class DefaultAxes
{
    /// <summary>创建默认轴映射。</summary>
    public static Dictionary<string, AxisMapping> Create() => new()
    {
        ["Horizontal"] = new AxisMapping
        {
            PositiveKey = KeyCode.D,
            NegativeKey = KeyCode.A,
        },
        ["Vertical"] = new AxisMapping
        {
            PositiveKey = KeyCode.W,
            NegativeKey = KeyCode.S,
        },
    };
}
