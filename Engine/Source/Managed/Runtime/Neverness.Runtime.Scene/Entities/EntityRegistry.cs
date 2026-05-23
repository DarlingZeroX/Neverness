using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 实体注册表——管理 Native handle 与 Managed <see cref="SceneEntity"/> 的双向映射。
/// 由 <see cref="SceneWorld"/> 持有，不对外暴露 Native 句柄操作。
/// </summary>
public sealed class EntityRegistry
{
    /// <summary>所属 Native 场景句柄。</summary>
    private readonly ulong _sceneHandle;

    /// <summary>句柄 → 实体映射（O(1) 查找）。</summary>
    private readonly Dictionary<ulong, SceneEntity> _handleMap = new();

    /// <summary>所有存活实体的有序列表（保持遍历顺序稳定）。</summary>
    private readonly List<SceneEntity> _entities = [];

    /// <summary>当前注册的实体数量。</summary>
    public int Count => _entities.Count;

    /// <summary>所有实体的只读视图。</summary>
    public IReadOnlyList<SceneEntity> Entities => _entities;

    public EntityRegistry(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
    }

    /// <summary>创建实体并注册。</summary>
    /// <param name="displayName">显示名称。</param>
    /// <returns>创建的实体；无场景或创建失败时返回 null。</returns>
    public SceneEntity? Create(string? displayName = null)
    {
        if (_sceneHandle == 0)
        {
            return null;
        }

        var handle = SceneNativeBridge.CreateEntity(_sceneHandle);
        if (handle.Value == 0)
        {
            return null;
        }

        var entity = new SceneEntity(handle, _sceneHandle, displayName ?? "Entity");
        Register(entity);
        return entity;
    }

    /// <summary>销毁实体并解除注册。</summary>
    public NNSceneResult Destroy(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);

        var result = SceneNativeBridge.DestroyEntity(_sceneHandle, entity.Handle);
        if (result == NNSceneResult.Ok)
        {
            Unregister(entity);
            entity.Invalidate();
        }

        return result;
    }

    /// <summary>注册已有实体（如 Prefab 实例化后的实体）。</summary>
    public void Register(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        if (_handleMap.TryAdd(entity.Handle.Value, entity))
        {
            _entities.Add(entity);
        }
    }

    /// <summary>解除注册（不销毁 Native 实体）。</summary>
    public void Unregister(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        if (_handleMap.Remove(entity.Handle.Value))
        {
            _entities.Remove(entity);
        }
    }

    /// <summary>按句柄查找实体。</summary>
    public SceneEntity? Find(NNEntityHandle handle)
    {
        _handleMap.TryGetValue(handle.Value, out var entity);
        return entity;
    }

    /// <summary>按句柄获取实体（找不到时抛异常）。</summary>
    public SceneEntity Get(NNEntityHandle handle)
    {
        if (_handleMap.TryGetValue(handle.Value, out var entity))
        {
            return entity;
        }

        throw new KeyNotFoundException($"Entity with handle {handle.Value} not found in registry.");
    }

    /// <summary>尝试获取实体。</summary>
    public bool TryGet(NNEntityHandle handle, out SceneEntity? entity)
    {
        return _handleMap.TryGetValue(handle.Value, out entity);
    }

    /// <summary>是否包含指定实体。</summary>
    public bool Contains(SceneEntity entity) =>
        entity != null && _handleMap.ContainsKey(entity.Handle.Value);

    /// <summary>清理所有注册（不销毁 Native 实体）。</summary>
    public void Clear()
    {
        _handleMap.Clear();
        _entities.Clear();
    }

    /// <summary>
    /// 从快照恢复注册表（热重载使用）。
    /// 接收 Native 实体句柄列表，为每个句柄重建 Managed SceneEntity 映射。
    /// </summary>
    /// <param name="handles">存活的 Native 实体句柄列表。</param>
    public void SyncFromHandles(IReadOnlyList<ulong> handles)
    {
        _handleMap.Clear();
        _entities.Clear();

        foreach (var handleValue in handles)
        {
            if (handleValue == 0)
            {
                continue;
            }

            var entity = new SceneEntity(
                new NNEntityHandle(handleValue), _sceneHandle, "Entity");
            _handleMap[handleValue] = entity;
            _entities.Add(entity);
        }
    }

    /// <summary>导出当前所有实体的 Native 句柄值（用于快照保存）。</summary>
    internal IReadOnlyList<ulong> ExportHandleValues()
    {
        var handles = new List<ulong>(_entities.Count);
        foreach (var entity in _entities)
        {
            handles.Add(entity.Handle.Value);
        }
        return handles;
    }
}
