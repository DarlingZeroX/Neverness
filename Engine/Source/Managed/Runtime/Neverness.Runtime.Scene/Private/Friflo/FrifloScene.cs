using System.Text.Json;
using Friflo.Engine.ECS;
using Neverness.Runtime.Scene.Components;
using Neverness.Runtime.Scene.Query;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// IScene 的 Friflo ECS 实现。
/// 内部使用 EntityStore，对外只暴露 IScene 接口。
/// </summary>
public sealed class FrifloScene : IScene
{
    private readonly EntityStore _store;
    private readonly List<ISceneSystem> _systems = new();
    private bool _disposed;

    // ── IScene 属性 ──

    public string Name { get; set; }
    public int EntityCount => _store.Count;
    public bool IsValid => !_disposed;
    public SceneEventBus Events { get; }

    // ── IScene 接口 ──
    ISceneEventBus IScene.Events => Events;

    // ── 构造 ──

    public FrifloScene(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        Name = name;
        _store = new EntityStore();
        Events = new SceneEventBus();
    }

    // ── 实体操作 ──

    public IEntity CreateEntity(string? displayName = null)
    {
        ThrowIfDisposed();
        var entity = _store.CreateEntity();

        // 每个实体自带 TagComponent（名字）和 RelationshipComponent（层级）
        entity.AddComponent(TagComponent.Create(displayName ?? $"Entity_{entity.Id}"));
        entity.AddComponent(RelationshipComponent.Default);

        return new FrifloEntity(entity, this);
    }

    public void DestroyEntity(IEntity entity)
    {
        ThrowIfDisposed();
        if (entity is FrifloEntity frifloEntity)
        {
            frifloEntity.InternalEntity.DeleteEntity();
        }
    }

    public void DestroyAllEntities()
    {
        ThrowIfDisposed();
        // 创建新的 EntityStore 替换旧的
        // Friflo 没有直接清空所有实体的方法
        // TODO: 实现更高效的清空方法
    }

    public IEntity? GetEntity(int entityId)
    {
        ThrowIfDisposed();
        try
        {
            var entity = _store.GetEntityById(entityId);
            if (entity.IsNull)
                return null;
            return new FrifloEntity(entity, this);
        }
        catch (ArgumentException)
        {
            // Friflo 在 ID 超出范围时抛异常
            return null;
        }
    }

    public bool EntityExists(int entityId)
    {
        ThrowIfDisposed();
        try
        {
            var entity = _store.GetEntityById(entityId);
            return !entity.IsNull;
        }
        catch (ArgumentException)
        {
            return false;
        }
    }

    // ── 查询 ──

    public ISceneQuery<T1> Query<T1>()
        where T1 : struct, IComponent
    {
        ThrowIfDisposed();
        return new FrifloQuery<T1>(_store, this);
    }

    public ISceneQuery<T1, T2> Query<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
    {
        ThrowIfDisposed();
        return new FrifloQuery<T1, T2>(_store, this);
    }

    public ISceneQuery<T1, T2, T3> Query<T1, T2, T3>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
        where T3 : struct, IComponent
    {
        ThrowIfDisposed();
        return new FrifloQuery<T1, T2, T3>(_store, this);
    }

    // ── 视图 ──

    /// <summary>创建单组件视图。</summary>
    public SceneView<T> CreateView<T>() where T : struct, IComponent
    {
        ThrowIfDisposed();
        return new SceneView<T>(_store, this);
    }

    /// <summary>创建双组件视图。</summary>
    public SceneView<T1, T2> CreateView<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent
    {
        ThrowIfDisposed();
        return new SceneView<T1, T2>(_store, this);
    }

    // ── 系统管理 ──

    public void AddSystem(ISceneSystem system)
    {
        ThrowIfDisposed();
        ArgumentNullException.ThrowIfNull(system);

        if (!_systems.Contains(system))
        {
            _systems.Add(system);
            _systems.Sort((a, b) => a.Priority.CompareTo(b.Priority));

            if (IsValid)
            {
                system.Initialize(this);
            }
        }
    }

    public void RemoveSystem(ISceneSystem system)
    {
        ThrowIfDisposed();
        if (_systems.Remove(system))
        {
            system.Shutdown();
        }
    }

    public T? GetSystem<T>() where T : class
    {
        return _systems.OfType<T>().FirstOrDefault();
    }

    // ── 更新 ──

    public void Update(float deltaTime)
    {
        ThrowIfDisposed();
        foreach (var system in _systems)
        {
            if (system.IsInitialized)
            {
                system.Update(deltaTime);
            }
        }
        Events.FlushDeferred();
    }

    public void FixedUpdate(float fixedDeltaTime)
    {
        ThrowIfDisposed();
        foreach (var system in _systems)
        {
            if (system.IsInitialized)
            {
                system.FixedUpdate(fixedDeltaTime);
            }
        }
    }

    public void UpdateByTagMask(float deltaTime, SceneSystemTags mask)
    {
        ThrowIfDisposed();
        foreach (var system in _systems)
        {
            if (system.IsInitialized && (system.Tags & mask) != 0)
            {
                system.Update(deltaTime);
            }
        }
        Events.FlushDeferred();
    }

    // ── 层级操作 ──

    public void SetParent(IEntity child, IEntity parent)
    {
        ThrowIfDisposed();
        ArgumentNullException.ThrowIfNull(child);
        ArgumentNullException.ThrowIfNull(parent);

        if (!child.IsValid || !parent.IsValid)
            throw new ArgumentException("实体无效");

        if (child == parent)
            throw new ArgumentException("不能设置自己为自己的父节点");

        // 检查循环引用
        if (WouldCreateCycle(child, parent))
            throw new ArgumentException("检测到循环引用");

        if (child is FrifloEntity frifloChild && parent is FrifloEntity frifloParent)
        {
            // 从旧父节点移除
            var oldParent = GetParent(child);
            if (oldParent != null && oldParent.IsValid)
            {
                RemoveChildFromParent(oldParent, child);
            }

            // 设置新父节点
            if (frifloChild.InternalEntity.HasComponent<RelationshipComponent>())
            {
                ref var rel = ref frifloChild.InternalEntity.GetComponent<RelationshipComponent>();
                rel.ParentId = frifloParent.InternalEntity.Id;
            }
            else
            {
                frifloChild.InternalEntity.AddComponent(new RelationshipComponent
                {
                    ParentId = frifloParent.InternalEntity.Id,
                    ChildCount = 0,
                    Depth = 0,
                });
            }

            // 添加到新父节点的子列表
            AddChildToParent(parent, child);
        }
    }

    public IEntity? GetParent(IEntity entity)
    {
        ThrowIfDisposed();
        if (entity == null || !entity.IsValid) return null;

        if (entity is FrifloEntity frifloEntity)
        {
            if (frifloEntity.InternalEntity.HasComponent<RelationshipComponent>())
            {
                ref var rel = ref frifloEntity.InternalEntity.GetComponent<RelationshipComponent>();
                if (rel.ParentId >= 0)
                {
                    return GetEntity(rel.ParentId);
                }
            }
        }

        return null;
    }

    public IReadOnlyList<IEntity> GetChildren(IEntity entity)
    {
        ThrowIfDisposed();
        if (entity == null || !entity.IsValid)
            return Array.Empty<IEntity>();

        var children = new List<IEntity>();

        _store.Query<RelationshipComponent>().ForEachEntity((ref RelationshipComponent rel, Entity frifloEntity) =>
        {
            if (rel.ParentId == entity.Id)
            {
                var child = new FrifloEntity(frifloEntity, this);
                children.Add(child);
            }
        });

        return children;
    }

    // ── 层级辅助方法 ──

    private bool WouldCreateCycle(IEntity child, IEntity potentialParent)
    {
        var current = potentialParent;
        int depth = 0;
        while (current != null && current.IsValid)
        {
            // 使用 Id 进行比较，而不是引用相等性
            if (current.Id == child.Id) return true;
            current = GetParent(current);
            depth++;
            // 安全阀：防止无限循环
            if (depth > 1000)
            {
                System.Diagnostics.Debug.WriteLine("[FrifloScene] WouldCreateCycle: 层级深度超过 1000，可能存在循环");
                return true;
            }
        }
        return false;
    }

    private void AddChildToParent(IEntity parent, IEntity child)
    {
        if (parent is FrifloEntity frifloParent)
        {
            if (frifloParent.InternalEntity.HasComponent<RelationshipComponent>())
            {
                ref var rel = ref frifloParent.InternalEntity.GetComponent<RelationshipComponent>();
                rel.ChildCount++;
            }
            else
            {
                frifloParent.InternalEntity.AddComponent(new RelationshipComponent
                {
                    ParentId = -1,
                    ChildCount = 1,
                    Depth = 0,
                });
            }
        }
    }

    private void RemoveChildFromParent(IEntity parent, IEntity child)
    {
        if (parent is FrifloEntity frifloParent)
        {
            if (frifloParent.InternalEntity.HasComponent<RelationshipComponent>())
            {
                ref var rel = ref frifloParent.InternalEntity.GetComponent<RelationshipComponent>();
                if (rel.ChildCount > 0)
                    rel.ChildCount--;
            }
        }
    }

    // ── 序列化（调用 Source Generator 生成的 ComponentSerializer） ──

    public void Serialize(Stream stream, string format = "json")
    {
        ThrowIfDisposed();
        ArgumentNullException.ThrowIfNull(stream);

        if (format.ToLowerInvariant() != "json")
            throw new NotSupportedException($"不支持的序列化格式: {format}");

        using var writer = new Utf8JsonWriter(stream, new JsonWriterOptions
        {
            Indented = true,
            Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
        });

        writer.WriteStartObject();
        writer.WriteStartArray("entities");

        foreach (var entity in _store.Entities)
        {
            writer.WriteStartObject();

            // id
            writer.WriteNumber("id", entity.Id);

            // name（从 TagComponent 读取）
            string name = entity.HasComponent<TagComponent>()
                ? entity.GetComponent<TagComponent>().Name
                : $"Entity_{entity.Id}";
            writer.WriteString("name", name);

            // parentId（如果有 RelationshipComponent）
            if (entity.HasComponent<RelationshipComponent>())
            {
                ref var rel = ref entity.GetComponent<RelationshipComponent>();
                writer.WriteNumber("parentId", rel.ParentId);
            }

            // components（由 Generator 生成的 Write 方法）
            writer.WriteStartObject("components");

            if (entity.HasComponent<TransformComponent>())
            {
                ref var c = ref entity.GetComponent<TransformComponent>();
                ComponentSerializer.WriteTransformComponent(ref c, writer);
            }
            if (entity.HasComponent<CameraComponent>())
            {
                ref var c = ref entity.GetComponent<CameraComponent>();
                ComponentSerializer.WriteCameraComponent(ref c, writer);
            }
            if (entity.HasComponent<ScriptComponent>())
            {
                ref var c = ref entity.GetComponent<ScriptComponent>();
                ComponentSerializer.WriteScriptComponent(ref c, writer);
            }
            if (entity.HasComponent<SpriteRendererComponent>())
            {
                ref var c = ref entity.GetComponent<SpriteRendererComponent>();
                ComponentSerializer.WriteSpriteRendererComponent(ref c, writer);
            }
            if (entity.HasComponent<AudioSourceComponent>())
            {
                ref var c = ref entity.GetComponent<AudioSourceComponent>();
                ComponentSerializer.WriteAudioSourceComponent(ref c, writer);
            }
            if (entity.HasComponent<VideoPlayerComponent>())
            {
                ref var c = ref entity.GetComponent<VideoPlayerComponent>();
                ComponentSerializer.WriteVideoPlayerComponent(ref c, writer);
            }
            if (entity.HasComponent<RmlUIDocumentComponent>())
            {
                ref var c = ref entity.GetComponent<RmlUIDocumentComponent>();
                ComponentSerializer.WriteRmlUIDocumentComponent(ref c, writer);
            }

            writer.WriteEndObject(); // components
            writer.WriteEndObject(); // entity
        }

        writer.WriteEndArray(); // entities
        writer.WriteEndObject(); // root
    }

    public void Deserialize(Stream stream, string format = "json")
    {
        ThrowIfDisposed();
        ArgumentNullException.ThrowIfNull(stream);

        if (format.ToLowerInvariant() != "json")
            throw new NotSupportedException($"不支持的序列化格式: {format}");

        using var reader = new StreamReader(stream, System.Text.Encoding.UTF8);
        var json = reader.ReadToEnd();

        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;

        if (!root.TryGetProperty("entities", out var entitiesElement))
            return;

        // 第一遍：创建所有实体 + 添加组件
        var idMap = new Dictionary<int, Entity>();

        foreach (var entityElem in entitiesElement.EnumerateArray())
        {
            int id = entityElem.GetProperty("id").GetInt32();
            string name = entityElem.TryGetProperty("name", out var nameElem)
                ? nameElem.GetString() ?? $"Entity_{id}"
                : $"Entity_{id}";

            // CreateEntity 自带 TagComponent + RelationshipComponent
            var wrapped = (FrifloEntity)CreateEntity(name);
            var entity = wrapped.InternalEntity;
            idMap[id] = entity;

            // 添加组件（由 Generator 生成的 TryAddComponent 分发）
            if (!entityElem.TryGetProperty("components", out var compsElem))
                continue;

            foreach (var compProp in compsElem.EnumerateObject())
            {
                ComponentSerializer.TryAddComponent(entity, compProp.Name, compProp.Value);
            }
        }

        // 第二遍：重建父子层级
        foreach (var entityElem in entitiesElement.EnumerateArray())
        {
            if (!entityElem.TryGetProperty("parentId", out var parentIdElem))
                continue;

            int parentId = parentIdElem.GetInt32();
            if (parentId < 0)
                continue;

            int childId = entityElem.GetProperty("id").GetInt32();
            if (idMap.TryGetValue(childId, out var child) && idMap.TryGetValue(parentId, out var parent))
            {
                var wrappedChild = new FrifloEntity(child, this);
                var wrappedParent = new FrifloEntity(parent, this);
                SetParent(wrappedChild, wrappedParent);
            }
        }
    }

    // ── 热重载 ──

    /// <summary>保存热重载快照。</summary>
    public HotReloadSnapshot SaveSnapshot()
    {
        var snapshot = new HotReloadSnapshot
        {
            Name = Name,
            CapturedAt = DateTimeOffset.UtcNow,
        };

        // 收集所有实体 ID（使用 EntityStore 的实体枚举）
        foreach (var entity in _store.Entities)
        {
            snapshot.EntityIds.Add(entity.Id);
        }

        return snapshot;
    }

    // ── 释放 ──

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        // 关闭系统（逆序）
        for (int i = _systems.Count - 1; i >= 0; i--)
        {
            _systems[i].Shutdown();
            _systems[i].Dispose();
        }
        _systems.Clear();

        // 清空事件总线
        Events.Clear();
    }

    // ── 内部方法 ──

    /// <summary>底层 Friflo EntityStore（内部使用）。</summary>
    public EntityStore Store => _store;

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }

    // 空组件用于查询所有实体
    private struct _component : IComponent { }
}
