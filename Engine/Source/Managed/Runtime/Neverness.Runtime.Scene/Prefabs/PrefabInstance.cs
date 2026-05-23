using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Prefab 实例——运行时实体子图与源 Prefab 的关联。
/// 跟踪实例化后的实体映射和属性覆盖。
/// </summary>
public sealed class PrefabInstance
{
    /// <summary>源 Prefab 引用。</summary>
    public PrefabAsset Source { get; }

    /// <summary>根实体句柄。</summary>
    public NNEntityHandle RootEntity { get; internal set; }

    /// <summary>Prefab 局部索引 → 运行时实体句柄映射。</summary>
    public IReadOnlyDictionary<int, NNEntityHandle> InstanceMap => _instanceMap;

    /// <summary>属性覆盖列表。</summary>
    public IReadOnlyList<PrefabOverride> Overrides => _overrides;

    private readonly Dictionary<int, NNEntityHandle> _instanceMap = new();
    private readonly List<PrefabOverride> _overrides = [];

    public PrefabInstance(PrefabAsset source)
    {
        ArgumentNullException.ThrowIfNull(source);
        Source = source;
    }

    /// <summary>按局部索引获取运行时实体句柄。</summary>
    public NNEntityHandle GetEntity(int localIndex)
    {
        _instanceMap.TryGetValue(localIndex, out var handle);
        return handle;
    }

    /// <summary>记录覆盖。</summary>
    public void AddOverride(PrefabOverride override_)
    {
        ArgumentNullException.ThrowIfNull(override_);
        _overrides.Add(override_);
    }

    /// <summary>清除所有覆盖。</summary>
    public void ClearOverrides()
    {
        _overrides.Clear();
    }

    internal void SetInstanceMap(Dictionary<int, NNEntityHandle> map)
    {
        _instanceMap.Clear();
        foreach (var (k, v) in map)
        {
            _instanceMap[k] = v;
        }
    }
}
