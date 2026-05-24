using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 查询对象缓存——避免重复创建 Query 对象。
/// 由 <see cref="SceneWorld"/> 持有，按组件 TypeId 缓存。
/// </summary>
public sealed class SceneQueryCache
{
    /// <summary>所属 Native 场景句柄。</summary>
    private readonly ulong _sceneHandle;

    /// <summary>TypeId → 缓存查询对象映射。</summary>
    private readonly Dictionary<ulong, object> _cache = new();

    public SceneQueryCache(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
    }

    /// <summary>获取场景句柄。</summary>
    public ulong SceneHandle => _sceneHandle;

    /// <summary>获取或创建单组件查询。</summary>
    public SceneQuery<T> GetQuery<T>() where T : unmanaged
    {
        var key = ComponentTypeCache<T>.TypeId;
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T>(_sceneHandle);
            _cache[key] = query;
        }
        return (SceneQuery<T>)query;
    }

    /// <summary>获取或创建双组件查询。</summary>
    public SceneQuery<T1, T2> GetQuery<T1, T2>()
        where T1 : unmanaged
        where T2 : unmanaged
    {
        // 双组件查询使用组合键（避免与单组件缓存冲突）
        var key1 = ComponentTypeCache<T1>.TypeId;
        var key2 = ComponentTypeCache<T2>.TypeId;
        var key = key1 ^ (key2 * 31);
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T1, T2>(_sceneHandle);
            _cache[key] = query;
        }
        return (SceneQuery<T1, T2>)query;
    }

    /// <summary>清理所有缓存查询。</summary>
    public void Clear()
    {
        _cache.Clear();
    }
}
