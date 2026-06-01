// ============================================================================
// ColliderComponent.cs - 碰撞体组件
// ============================================================================
// 碰撞体组件，定义碰撞形状和物理材质参数。
// ============================================================================

using System.Runtime.InteropServices;

namespace Neverness.Gameplay;

/// <summary>
/// 碰撞体组件（基础数据）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct ColliderComponent
{
    /// <summary>是否为触发器（不产生物理碰撞，只触发事件）。</summary>
    public byte IsTrigger;  // byte 对齐 C++ bool

    /// <summary>是否启用。</summary>
    public byte IsEnabled;

    /// <summary>物理材质 GUID（高位）。</summary>
    public ulong PhysicMaterialGuidHigh;

    /// <summary>物理材质 GUID（低位）。</summary>
    public ulong PhysicMaterialGuidLow;

    /// <summary>碰撞层。</summary>
    public uint Layer;

    /// <summary>碰撞层掩码。</summary>
    public uint LayerMask;

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>默认碰撞体组件。</summary>
    public static readonly ColliderComponent Default = new()
    {
        IsTrigger = 0,
        IsEnabled = 1,
        PhysicMaterialGuidHigh = 0,
        PhysicMaterialGuidLow = 0,
        Layer = 0,
        LayerMask = 0xFFFFFFFF
    };
}

/// <summary>
/// 盒子碰撞体组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct BoxColliderComponent
{
    /// <summary>基础碰撞体数据。</summary>
    public ColliderComponent Base;

    /// <summary>中心偏移。</summary>
    public Vector3 Center;

    /// <summary>大小。</summary>
    public Vector3 Size;
}

/// <summary>
/// 球体碰撞体组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct SphereColliderComponent
{
    /// <summary>基础碰撞体数据。</summary>
    public ColliderComponent Base;

    /// <summary>中心偏移。</summary>
    public Vector3 Center;

    /// <summary>半径。</summary>
    public float Radius;
}

/// <summary>
/// 胶囊碰撞体组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct CapsuleColliderComponent
{
    /// <summary>基础碰撞体数据。</summary>
    public ColliderComponent Base;

    /// <summary>中心偏移。</summary>
    public Vector3 Center;

    /// <summary>半径。</summary>
    public float Radius;

    /// <summary>高度。</summary>
    public float Height;

    /// <summary>方向（0=X, 1=Y, 2=Z）。</summary>
    public int Direction;
}

/// <summary>
/// 网格碰撞体组件。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct MeshColliderComponent
{
    /// <summary>基础碰撞体数据。</summary>
    public ColliderComponent Base;

    /// <summary>网格资源 GUID（高位）。</summary>
    public ulong MeshGuidHigh;

    /// <summary>网格资源 GUID（低位）。</summary>
    public ulong MeshGuidLow;

    /// <summary>是否为凸包。</summary>
    public byte IsConvex;
}
