namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 覆盖类型。
/// </summary>
public enum PrefabOverrideType : byte
{
    /// <summary>组件属性值被修改。</summary>
    PropertyModified = 0,

    /// <summary>添加了新组件。</summary>
    ComponentAdded = 1,

    /// <summary>移除了组件。</summary>
    ComponentRemoved = 2,

    /// <summary>添加了子实体。</summary>
    ChildAdded = 3,

    /// <summary>移除了子实体。</summary>
    ChildRemoved = 4,
}

/// <summary>
/// 单条 Prefab 覆盖记录。
/// </summary>
public sealed class PrefabOverride
{
    /// <summary>覆盖类型。</summary>
    public PrefabOverrideType Type { get; init; }

    /// <summary>受影响的实体局部索引。</summary>
    public int EntityLocalIndex { get; init; }

    /// <summary>受影响的组件 TypeId（PropertyModified / ComponentAdded / ComponentRemoved）。</summary>
    public ulong ComponentTypeId { get; init; }

    /// <summary>属性路径（PropertyModified 时使用）。</summary>
    public string? PropertyPath { get; init; }

    /// <summary>新值（PropertyModified 时使用）。</summary>
    public byte[]? NewValue { get; init; }
}
