// ============================================================================
// MeshRendererComponent.cs - 网格渲染组件
// ============================================================================
// 网格渲染组件，定义网格、材质等渲染参数。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 网格渲染组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct MeshRendererComponent
{
    /// <summary>网格资源 GUID（高位）。</summary>
    public ulong MeshGuidHigh;

    /// <summary>网格资源 GUID（低位）。</summary>
    public ulong MeshGuidLow;

    /// <summary>材质资源 GUID（高位）。</summary>
    public ulong MaterialGuidHigh;

    /// <summary>材质资源 GUID（低位）。</summary>
    public ulong MaterialGuidLow;

    /// <summary>是否投射阴影。</summary>
    public byte CastShadows;  // byte 对齐 C++ bool

    /// <summary>是否接收阴影。</summary>
    public byte ReceiveShadows;

    /// <summary>是否可见。</summary>
    public byte IsVisible;

    /// <summary>渲染层掩码。</summary>
    public uint LayerMask;

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认网格渲染组件。</summary>
    public static readonly MeshRendererComponent Default = new()
    {
        MeshGuidHigh = 0,
        MeshGuidLow = 0,
        MaterialGuidHigh = 0,
        MaterialGuidLow = 0,
        CastShadows = 1,
        ReceiveShadows = 1,
        IsVisible = 1,
        LayerMask = 0xFFFFFFFF
    };
}
