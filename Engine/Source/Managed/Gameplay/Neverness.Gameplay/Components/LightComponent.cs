// ============================================================================
// LightComponent.cs - 灯光组件
// ============================================================================
// 灯光组件，定义灯光类型、颜色、强度等参数。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 灯光组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct LightComponent
{
    /// <summary>灯光类型。</summary>
    public LightType Type;

    /// <summary>灯光颜色。</summary>
    public Vector3 Color;

    /// <summary>灯光强度。</summary>
    public float Intensity;

    /// <summary>范围（点光/聚光）。</summary>
    public float Range;

    /// <summary>聚光角度（度）。</summary>
    public float SpotAngle;

    /// <summary>内聚光角度（度）。</summary>
    public float InnerSpotAngle;

    /// <summary>是否投射阴影。</summary>
    public byte CastShadows;  // byte 对齐 C++ bool

    /// <summary>阴影强度。</summary>
    public float ShadowStrength;

    /// <summary>阴影分辨率。</summary>
    public int ShadowResolution;

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认方向光。</summary>
    public static readonly LightComponent DefaultDirectional = new()
    {
        Type = LightType.Directional,
        Color = Vector3.One,
        Intensity = 1f,
        Range = 10f,
        SpotAngle = 30f,
        InnerSpotAngle = 25f,
        CastShadows = 1,
        ShadowStrength = 1f,
        ShadowResolution = 2048
    };

    /// <summary>默认点光源。</summary>
    public static readonly LightComponent DefaultPoint = new()
    {
        Type = LightType.Point,
        Color = Vector3.One,
        Intensity = 1f,
        Range = 10f,
        SpotAngle = 30f,
        InnerSpotAngle = 25f,
        CastShadows = 1,
        ShadowStrength = 1f,
        ShadowResolution = 1024
    };
}

/// <summary>
/// 灯光类型。
/// </summary>
public enum LightType
{
    /// <summary>方向光。</summary>
    Directional = 0,

    /// <summary>点光源。</summary>
    Point = 1,

    /// <summary>聚光灯。</summary>
    Spot = 2,

    /// <summary>区域光。</summary>
    Area = 3
}
