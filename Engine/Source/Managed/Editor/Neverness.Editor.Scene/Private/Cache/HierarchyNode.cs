namespace Neverness.Editor.Scene.Private.Cache;

/// <summary>
/// 层级缓存节点——从 NNSceneNodeSnapshot 解析而来。
/// 仅在 SceneHierarchyCache 内部使用，不对外暴露 Native 结构体。
/// </summary>
public sealed class HierarchyNode
{
    /// <summary>实体 ID。</summary>
    public int EntityId;

    /// <summary>父实体 ID（根节点 = -1）。</summary>
    public int Parent = -1;

    /// <summary>DFS 深度（根 = 0）。</summary>
    public uint Depth;

    /// <summary>直接子节点数。</summary>
    public uint ChildCount;

    /// <summary>实体名称（从 namePool 解析的 UTF-8 字符串）。</summary>
    public string Name = "";

    /// <summary>实体标志位（bit0=active, bit1=prefab_instance, bit2=dirty, bit3=selected）。</summary>
    public uint Flags;

    /// <summary>在 SceneHierarchyCache._allNodes 中的索引。</summary>
    public int FlatIndex;

    /// <summary>是否活跃（Flags bit0）。</summary>
    public bool IsActive => (Flags & 0x01) != 0;

    /// <summary>是否为 Prefab 实例（Flags bit1）。</summary>
    public bool IsPrefabInstance => (Flags & 0x02) != 0;

    /// <summary>是否有未保存修改（Flags bit2）。</summary>
    public bool IsDirty => (Flags & 0x04) != 0;

    /// <summary>是否被选中（Flags bit3）。</summary>
    public bool IsFlagSelected => (Flags & 0x08) != 0;
}
