namespace Neverness.Runtime.Scene.Prefab;

/// <summary>
/// 预制体管理器——管理预制体资产的注册、缓存和加载。
/// </summary>
public sealed class PrefabManager
{
    private readonly Dictionary<string, PrefabAsset> _prefabsByName = new();
    private readonly Dictionary<Guid, PrefabAsset> _prefabsByGuid = new();

    /// <summary>已注册的预制体数量。</summary>
    public int Count => _prefabsByName.Count;

    /// <summary>注册预制体。</summary>
    public void Register(PrefabAsset prefab)
    {
        ArgumentNullException.ThrowIfNull(prefab);
        _prefabsByName[prefab.Name] = prefab;
        _prefabsByGuid[prefab.Guid] = prefab;
    }

    /// <summary>注销预制体。</summary>
    public bool Unregister(string name)
    {
        if (_prefabsByName.TryGetValue(name, out var prefab))
        {
            _prefabsByName.Remove(name);
            _prefabsByGuid.Remove(prefab.Guid);
            return true;
        }
        return false;
    }

    /// <summary>按名称获取预制体。</summary>
    public PrefabAsset? GetByName(string name)
    {
        return _prefabsByName.GetValueOrDefault(name);
    }

    /// <summary>按 GUID 获取预制体。</summary>
    public PrefabAsset? GetByGuid(Guid guid)
    {
        return _prefabsByGuid.GetValueOrDefault(guid);
    }

    /// <summary>检查预制体是否存在。</summary>
    public bool Exists(string name)
    {
        return _prefabsByName.ContainsKey(name);
    }

    /// <summary>获取所有预制体名称。</summary>
    public IReadOnlyCollection<string> GetAllNames()
    {
        return _prefabsByName.Keys;
    }

    /// <summary>获取所有预制体。</summary>
    public IReadOnlyCollection<PrefabAsset> GetAll()
    {
        return _prefabsByName.Values;
    }

    /// <summary>清空所有预制体。</summary>
    public void Clear()
    {
        _prefabsByName.Clear();
        _prefabsByGuid.Clear();
    }

    /// <summary>从文件加载预制体。</summary>
    public PrefabAsset? LoadFromFile(string filePath)
    {
        var prefab = PrefabSerializer.LoadFromFile(filePath);
        if (prefab != null)
        {
            Register(prefab);
        }
        return prefab;
    }

    /// <summary>保存预制体到文件。</summary>
    public void SaveToFile(string name, string filePath)
    {
        var prefab = GetByName(name);
        if (prefab == null)
            throw new ArgumentException($"预制体 '{name}' 不存在");

        PrefabSerializer.SaveToFile(prefab, filePath);
    }
}
