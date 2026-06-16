using System.Runtime.InteropServices;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// 关系组件——父子层级。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct RelationshipComponent : IComponent
{
    /// <summary>父实体 ID（-1 = 无父节点，即根节点）。</summary>
    public int ParentId;

    /// <summary>子实体数量。</summary>
    public uint ChildCount;

    /// <summary>层级深度（根节点 = 0）。</summary>
    public uint Depth;

    /// <summary>创建默认关系组件（无父、无子、深度 0）。</summary>
    public static RelationshipComponent Default => new()
    {
        ParentId = -1,
        ChildCount = 0,
        Depth = 0,
    };
}
