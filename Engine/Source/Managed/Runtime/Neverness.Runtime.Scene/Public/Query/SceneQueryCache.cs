using Friflo.Engine.ECS;

namespace Neverness.Runtime.Scene.Query;

/// <summary>
/// 查询对象缓存——避免重复创建 Query 对象。
/// 由 SceneWorld 持有，按组件类型缓存。
/// </summary>
public sealed class SceneQueryCache
{
    private readonly EntityStore _store;
    private readonly IScene _scene;

    /// <summary>类型组合键 → 缓存查询对象映射。</summary>
    private readonly Dictionary<long, object> _cache = new();

    public SceneQueryCache(EntityStore store, IScene scene)
    {
        _store = store;
        _scene = scene;
    }

    /// <summary>获取或创建单组件查询。</summary>
    public SceneQuery<T> GetQuery<T>() where T : struct, IComponent
    {
        var key = GetTypeKey<T>();
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T>(_store, _scene);
            _cache[key] = query;
        }
        return (SceneQuery<T>)query;
    }

    /// <summary>获取或创建双组件查询。</summary>
    public SceneQuery<T1, T2> GetQuery<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
    {
        var key = GetTypeKey<T1, T2>();
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T1, T2>(_store, _scene);
            _cache[key] = query;
        }
        return (SceneQuery<T1, T2>)query;
    }

    /// <summary>获取或创建三组件查询。</summary>
    public SceneQuery<T1, T2, T3> GetQuery<T1, T2, T3>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
        where T3 : struct, IComponent
    {
        var key = GetTypeKey<T1, T2, T3>();
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T1, T2, T3>(_store, _scene);
            _cache[key] = query;
        }
        return (SceneQuery<T1, T2, T3>)query;
    }

    /// <summary>清理所有缓存查询。</summary>
    public void Clear()
    {
        _cache.Clear();
    }

    /// <summary>获取缓存的查询数量。</summary>
    public int Count => _cache.Count;

    // ── 内部方法 ──

    private static long GetTypeKey<T>()
    {
        return typeof(T).GetHashCode();
    }

    private static long GetTypeKey<T1, T2>()
    {
        var h1 = typeof(T1).GetHashCode();
        var h2 = typeof(T2).GetHashCode();
        return h1 ^ (h2 * 31);
    }

    private static long GetTypeKey<T1, T2, T3>()
    {
        var h1 = typeof(T1).GetHashCode();
        var h2 = typeof(T2).GetHashCode();
        var h3 = typeof(T3).GetHashCode();
        return h1 ^ (h2 * 31) ^ (h3 * 37);
    }
}
