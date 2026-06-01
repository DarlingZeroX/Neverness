// ============================================================================
// CameraComponent.cs - 相机组件
// ============================================================================
// 相机组件，定义视场角、裁剪面等参数。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 相机组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct CameraComponent
{
    /// <summary>视场角（度）。</summary>
    public float FieldOfView;

    /// <summary>近裁剪面。</summary>
    public float NearClipPlane;

    /// <summary>远裁剪面。</summary>
    public float FarClipPlane;

    /// <summary>宽高比。</summary>
    public float AspectRatio;

    /// <summary>是否为正交投影。</summary>
    public byte IsOrthographic;  // byte 对齐 C++ bool

    /// <summary>正交投影大小。</summary>
    public float OrthographicSize;

    /// <summary>深度优先级。</summary>
    public int Depth;

    /// <summary>清除标志。</summary>
    public CameraClearFlags ClearFlags;

    /// <summary>背景颜色。</summary>
    public Vector4 BackgroundColor;

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认相机组件。</summary>
    public static readonly CameraComponent Default = new()
    {
        FieldOfView = 60f,
        NearClipPlane = 0.1f,
        FarClipPlane = 1000f,
        AspectRatio = 16f / 9f,
        IsOrthographic = 0,
        OrthographicSize = 5f,
        Depth = 0,
        ClearFlags = CameraClearFlags.Skybox,
        BackgroundColor = new Vector4(0.19f, 0.30f, 0.47f, 1f)
    };
}

/// <summary>
/// 相机清除标志。
/// </summary>
public enum CameraClearFlags
{
    /// <summary>天空盒。</summary>
    Skybox = 0,

    /// <summary>纯色。</summary>
    SolidColor = 1,

    /// <summary>仅深度。</summary>
    DepthOnly = 2,

    /// <summary>不清除。</summary>
    Nothing = 3
}
