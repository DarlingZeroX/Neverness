using System.Numerics;
using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 精灵混合模式（与 Native NNBlendMode 对齐）。
/// </summary>
public enum BlendMode : uint
{
    Alpha = 0,
    Additive = 1,
    Multiply = 2,
    Opaque = 3,
    Premultiplied = 4,
}

/// <summary>
/// 精灵渲染标志位（与 Native NNSpriteFlags 对齐）。
/// </summary>
[Flags]
public enum SpriteFlags : uint
{
    None = 0,
    Visible = 1 << 0,
    FlipX = 1 << 1,
    FlipY = 1 << 2,
    CastShadow = 1 << 3,
    ReceiveShadow = 1 << 4,
    Instanced = 1 << 5,
    CustomShader = 1 << 6,
}

/// <summary>
/// 精灵渲染组件——2D 渲染的核心组件。
/// 替代 C++ NNSpriteRendererComponent（88 字节）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct SpriteRendererComponent : IComponent
{
    /// <summary>纹理资产 GUID。</summary>
    public NNGuid TextureAsset;

    /// <summary>材质资产 GUID。</summary>
    public NNGuid MaterialAsset;

    /// <summary>纹理运行时 ID（瞬态，不序列化）。</summary>
    [Transient]
    public uint TextureRuntimeId;

    /// <summary>颜色 R 通道。</summary>
    public float ColorR;

    /// <summary>颜色 G 通道。</summary>
    public float ColorG;

    /// <summary>颜色 B 通道。</summary>
    public float ColorB;

    /// <summary>颜色 A 通道。</summary>
    public float ColorA;

    /// <summary>UV 矩形 U0。</summary>
    public float UvU0;

    /// <summary>UV 矩形 V0。</summary>
    public float UvV0;

    /// <summary>UV 矩形 U1。</summary>
    public float UvU1;

    /// <summary>UV 矩形 V1。</summary>
    public float UvV1;

    /// <summary>渲染层。</summary>
    public uint Layer;

    /// <summary>排序顺序。</summary>
    public uint SortOrder;

    /// <summary>混合模式。</summary>
    public BlendMode Blend;

    /// <summary>精灵标志位。</summary>
    public SpriteFlags Flags;

    /// <summary>颜色便捷属性。</summary>
    public Vector4 Color
    {
        readonly get => new(ColorR, ColorG, ColorB, ColorA);
        set { ColorR = value.X; ColorG = value.Y; ColorB = value.Z; ColorA = value.W; }
    }

    /// <summary>UV 矩形便捷属性。</summary>
    public Vector4 UvRect
    {
        readonly get => new(UvU0, UvV0, UvU1, UvV1);
        set { UvU0 = value.X; UvV0 = value.Y; UvU1 = value.Z; UvV1 = value.W; }
    }

    /// <summary>创建默认精灵渲染组件。</summary>
    public static SpriteRendererComponent Default => new()
    {
        ColorR = 1f, ColorG = 1f, ColorB = 1f, ColorA = 1f,
        UvU0 = 0f, UvV0 = 0f, UvU1 = 1f, UvV1 = 1f,
        Blend = BlendMode.Alpha,
        Flags = SpriteFlags.Visible,
    };
}
