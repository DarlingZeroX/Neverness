namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 中单个实体的描述——纯数据容器，无运行时状态。
/// </summary>
public sealed class PrefabEntityData
{
    /// <summary>在 Prefab 内的局部索引（0 = 根实体）。</summary>
    public int LocalIndex { get; init; }

    /// <summary>父实体的局部索引（-1 = 无父 / 根下实体）。</summary>
    public int ParentIndex { get; init; } = -1;

    /// <summary>显示名称。</summary>
    public string DisplayName { get; init; } = "Entity";

    /// <summary>组件数据（TypeId → 二进制 blob）。</summary>
    public Dictionary<ulong, byte[]> Components { get; } = new();
}
