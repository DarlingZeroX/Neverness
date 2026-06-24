using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene.Internal;
using Neverness.Runtime.Scene.Query;
using Neverness.Runtime.Scene.Systems;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景世界——Gameplay Runtime Root。
/// 持有一个 IScene 的完整 Managed 映射，管理该场景中的所有实体、查询和事件。
/// 兼容旧的 SceneWorld API，内部使用 FrifloScene 实现。
/// </summary>
public sealed class SceneWorld : IScene
{
    private bool _disposed;
    private readonly FrifloScene _scene;

    // ── 核心标识 ──

    /// <summary>场景名称。</summary>
    public string Name
    {
        get => _scene.Name;
        set => _scene.Name = value;
    }

    /// <summary>场景资产 GUID（可选，用于序列化和热重载）。</summary>
    public NNGuid AssetGuid { get; set; }

    /// <summary>场景资产 VFSService 路径（null = 未保存的新场景）。</summary>
    public string? AssetPath { get; set; }

    // ── 子系统 ──

    /// <summary>实体注册表（兼容旧 API）。</summary>
    public EntityRegistry Entities { get; }

    /// <summary>查询缓存（兼容旧 API）。</summary>
    public SceneQueryCache Queries { get; }

    /// <summary>Managed System 调度器（兼容旧 API）。</summary>
    public SceneSystemScheduler Systems { get; }

    /// <summary>场景事件总线。</summary>
    public SceneEventBus Events => (SceneEventBus)_scene.Events;

    /// <summary>底层 IScene 接口。</summary>
    public IScene Scene => _scene;

    /// <summary>是否有效（未释放）。</summary>
    public bool IsValid => !_disposed;

    /// <summary>本场景跟踪的实体数量。</summary>
    public int EntityCount => _scene.EntityCount;

    // ── 构造 ──

    private SceneWorld(string name)
    {
        _scene = new FrifloScene(name);
        Entities = new EntityRegistry(_scene);
        Queries = new SceneQueryCache(_scene.Store, _scene);
        Systems = new SceneSystemScheduler(_scene);

        // 注册核心 ECS 系统（每个场景都需要）
        // 优先级顺序：HierarchySystem(10) → SceneUpdateSystem(50) → TransformSystem(100) → CameraSystem(200)
        Systems.Register(new HierarchySystem());
        Systems.Register(new SceneUpdateSystem());
        Systems.Register(new TransformSystem());
        Systems.Register(new CameraSystem());
    }

    // ── 工厂方法 ──

    /// <summary>创建空场景世界。</summary>
    public static SceneWorld? Create(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        return new SceneWorld(name);
    }

    /// <summary>从 VFSService 资产路径加载并创建场景世界。</summary>
    public static SceneWorld? LoadFromAsset(string name, string vfsPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        Console.WriteLine($"[SceneWorld] LoadFromAsset: name={name}, vfsPath={vfsPath}");

        var world = new SceneWorld(name);

        try
        {
            // 诊断：检查 VFSService 路径解析
            var absPath = Neverness.Runtime.VFS.VFSService.GetAbsolutePath(vfsPath);
            Console.WriteLine($"[SceneWorld] VFSService 路径解析: {vfsPath} → {absPath ?? "null"}");

            // 诊断：检查 VFSService alias 是否注册
            var assetsRegistered = Neverness.Runtime.VFS.VFSService.IsAliasRegistered("/assets/");
            Console.WriteLine($"[SceneWorld] /assets/ alias 已注册: {assetsRegistered}");

            var json = Neverness.Runtime.VFS.VFSService.ReadText(vfsPath);
            if (json != null)
            {
                Console.WriteLine($"[SceneWorld] VFSService 读取成功: {json.Length} 字符");
                using var stream = new System.IO.MemoryStream(Encoding.UTF8.GetBytes(json));
                world._scene.Deserialize(stream, "json");
                Console.WriteLine($"[SceneWorld] 反序列化成功: EntityCount={world.EntityCount}");
            }
            else
            {
                Console.Error.WriteLine($"[SceneWorld] VFSService.ReadText 返回 null: {vfsPath}");
            }
            world.AssetPath = vfsPath;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[SceneWorld] 从 VFSService 加载场景失败: {ex.Message}");
            world.Dispose();
            return null;
        }

        return world;
    }

    // ── 实体操作 ──

    /// <summary>创建实体（仅基础组件，不发射事件）。
    /// 调用方应在组件挂载完毕后手动调用 EmitEntityCreated。</summary>
    public SceneEntity? CreateEntity(string? displayName = null)
    {
        var entity = _scene.CreateEntity(displayName);
        return new SceneEntity(entity, _scene);
    }

    /// <summary>发射 EntityCreated 事件（组件挂载完毕后调用）。</summary>
    public void EmitEntityCreated(IEntity entity)
    {
        Events.Emit(SceneEvent.OnEntityCreated(entity));
    }

    /// <summary>IScene 接口：创建实体。</summary>
    IEntity IScene.CreateEntity(string? displayName) => CreateEntity(displayName)!;

    /// <summary>销毁实体。</summary>
    public bool DestroyEntity(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        if (!entity.IsAlive) return false;

        // 先发射事件（Destroy 后 entity 引用可能失效）
        Events.Emit(SceneEvent.OnEntityDestroyed(entity.Entity));
        entity.Entity.Destroy();
        return true;
    }

    /// <summary>IScene 接口：销毁实体。</summary>
    void IScene.DestroyEntity(IEntity entity) => _scene.DestroyEntity(entity);

    /// <summary>IScene 接口：销毁所有实体。</summary>
    void IScene.DestroyAllEntities() => _scene.DestroyAllEntities();

    /// <summary>IScene 接口：获取实体。</summary>
    IEntity? IScene.GetEntity(int entityId) => _scene.GetEntity(entityId);

    /// <summary>IScene 接口：检查实体是否存在。</summary>
    bool IScene.EntityExists(int entityId) => _scene.EntityExists(entityId);

    // ── Tick ──

    /// <summary>主帧 Tick——按 TickGroup 顺序驱动所有系统。</summary>
    public void Tick(float deltaTime)
    {
        if (_disposed) return;
        _scene.Update(deltaTime);
    }

    /// <summary>固定步长 Tick。</summary>
    public void FixedTick(float fixedDeltaTime)
    {
        if (_disposed) return;
        _scene.FixedUpdate(fixedDeltaTime);
    }

    /// <summary>按标签掩码 Tick。</summary>
    public void TickByTagMask(float deltaTime, SceneSystemTags mask)
    {
        if (_disposed) return;
        _scene.UpdateByTagMask(deltaTime, mask);
    }

    // ── 查询 ──

    /// <summary>获取或创建单组件查询。</summary>
    public SceneQuery<T> GetQuery<T>() where T : struct, IComponent =>
        Queries.GetQuery<T>();

    /// <summary>获取或创建双组件查询。</summary>
    public SceneQuery<T1, T2> GetQuery<T1, T2>()
        where T1 : struct, IComponent
        where T2 : struct, IComponent =>
        Queries.GetQuery<T1, T2>();

    /// <summary>IScene 接口：创建单组件查询。</summary>
    ISceneQuery<T1> IScene.Query<T1>() => _scene.Query<T1>();

    /// <summary>IScene 接口：创建双组件查询。</summary>
    ISceneQuery<T1, T2> IScene.Query<T1, T2>() => _scene.Query<T1, T2>();

    /// <summary>IScene 接口：创建三组件查询。</summary>
    ISceneQuery<T1, T2, T3> IScene.Query<T1, T2, T3>() => _scene.Query<T1, T2, T3>();

    /// <summary>IScene 接口：创建单组件视图。</summary>
    SceneView<T> IScene.CreateView<T>() => _scene.CreateView<T>();

    /// <summary>IScene 接口：创建双组件视图。</summary>
    SceneView<T1, T2> IScene.CreateView<T1, T2>() => _scene.CreateView<T1, T2>();

    /// <summary>IScene 接口：添加系统。</summary>
    void IScene.AddSystem(ISceneSystem system) => _scene.AddSystem(system);

    /// <summary>IScene 接口：移除系统。</summary>
    void IScene.RemoveSystem(ISceneSystem system) => _scene.RemoveSystem(system);

    /// <summary>IScene 接口：获取系统。</summary>
    T? IScene.GetSystem<T>() where T : class => _scene.GetSystem<T>();

    /// <summary>IScene 接口：主帧更新。</summary>
    void IScene.Update(float deltaTime) => Tick(deltaTime);

    /// <summary>IScene 接口：固定步长更新。</summary>
    void IScene.FixedUpdate(float fixedDeltaTime) => FixedTick(fixedDeltaTime);

    /// <summary>IScene 接口：按标签掩码更新。</summary>
    void IScene.UpdateByTagMask(float deltaTime, SceneSystemTags mask) => TickByTagMask(deltaTime, mask);

    /// <summary>IScene 接口：设置父子关系。</summary>
    void IScene.SetParent(IEntity child, IEntity parent) => _scene.SetParent(child, parent);

    /// <summary>IScene 接口：获取父实体。</summary>
    IEntity? IScene.GetParent(IEntity entity) => _scene.GetParent(entity);

    /// <summary>IScene 接口：获取子实体列表。</summary>
    IReadOnlyList<IEntity> IScene.GetChildren(IEntity entity) => _scene.GetChildren(entity);

    /// <summary>IScene 接口：序列化场景到流。</summary>
    void IScene.Serialize(Stream stream, string format) => _scene.Serialize(stream, format);

    /// <summary>IScene 接口：从流反序列化场景。</summary>
    void IScene.Deserialize(Stream stream, string format) => _scene.Deserialize(stream, format);

    /// <summary>IScene 接口：事件总线。</summary>
    ISceneEventBus IScene.Events => Events;

    /// <summary>IScene 接口：是否有效。</summary>
    bool IScene.IsValid => IsValid;

    // ── 序列化 ──

    /// <summary>从流反序列化场景。</summary>
    public void Deserialize(Stream stream, string format = "json")
    {
        _scene.Deserialize(stream, format);
    }

    /// <summary>序列化场景到流。</summary>
    public void Serialize(Stream stream, string format = "json")
    {
        _scene.Serialize(stream, format);
    }

    /// <summary>序列化场景到 VFSService 路径（JSON 格式）。</summary>
    public NNSceneResult Save(string vfsPath)
    {
        if (_disposed) return NNSceneResult.Invalid;
        ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

        try
        {
            using var stream = new System.IO.MemoryStream();
            _scene.Serialize(stream, "json");
            var json = Encoding.UTF8.GetString(stream.ToArray());
            return Neverness.Runtime.VFS.VFSService.WriteText(vfsPath, json)
                ? NNSceneResult.Ok
                : NNSceneResult.Failed;
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"[SceneWorld] 保存场景失败: {ex.Message}");
            return NNSceneResult.Failed;
        }
    }

    // ── 热重载 ──

    /// <summary>保存热重载状态快照。</summary>
    public HotReloadSnapshot SaveSnapshot()
    {
        return new HotReloadSnapshot
        {
            Name = Name,
            CapturedAt = DateTimeOffset.UtcNow,
        };
    }

    /// <summary>热重载后重建 Managed 侧状态。</summary>
    public void RebuildAfterReload()
    {
        Systems.Rebuild();
    }

    // ── 释放 ──

    /// <summary>释放场景世界及其所有资源。</summary>
    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        Systems.Dispose();
        _scene.Dispose();
    }

    /// <summary>静默 Dispose——释放资源但不触发事件。</summary>
    internal void DisposeQuiet()
    {
        if (_disposed) return;
        _disposed = true;

        Systems.Dispose();
        _scene.Dispose();
    }

    /// <summary>从快照恢复世界（内部使用）。</summary>
    internal static SceneWorld RestoreFromSnapshotInternal(string name, NNGuid assetGuid)
    {
        var world = new SceneWorld(name);
        world.AssetGuid = assetGuid;
        return world;
    }
}

/// <summary>
/// 场景系统调度器——兼容旧 API。
/// </summary>
public sealed class SceneSystemScheduler : IDisposable
{
    private readonly IScene _scene;
    private readonly List<ISceneSystem> _systems = new();

    public SceneSystemScheduler(IScene scene)
    {
        _scene = scene;
    }

    /// <summary>注册系统。</summary>
    public void Register(ISceneSystem system)
    {
        _scene.AddSystem(system);
        _systems.Add(system);
    }

    /// <summary>重建系统（热重载后调用）。</summary>
    public void Rebuild()
    {
        foreach (var system in _systems)
        {
            system.Shutdown();
        }
        _systems.Clear();
    }

    /// <summary>释放调度器。</summary>
    public void Dispose()
    {
        foreach (var system in _systems)
        {
            system.Shutdown();
            system.Dispose();
        }
        _systems.Clear();
    }
}

/// <summary>
/// 实体注册表——兼容旧 API。
/// </summary>
public sealed class EntityRegistry
{
    private readonly IScene _scene;
    private readonly Dictionary<int, SceneEntity> _entities = new();

    public EntityRegistry(IScene scene)
    {
        _scene = scene;
    }

    /// <summary>实体数量。</summary>
    public int Count => _scene.EntityCount;

    /// <summary>所有实体。</summary>
    public IReadOnlyList<SceneEntity> Entities => _entities.Values.ToList();

    /// <summary>创建实体。</summary>
    public SceneEntity? Create(string? displayName = null)
    {
        var entity = _scene.CreateEntity(displayName);
        var sceneEntity = new SceneEntity(entity, _scene);
        _entities[entity.Id] = sceneEntity;
        return sceneEntity;
    }

    /// <summary>销毁实体。</summary>
    public bool Destroy(SceneEntity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);
        if (!entity.IsAlive) return false;

        entity.Entity.Destroy();
        _entities.Remove(entity.Id);
        return true;
    }

    /// <summary>注册实体。</summary>
    public void Register(SceneEntity entity)
    {
        _entities[entity.Id] = entity;
    }

    /// <summary>是否包含实体。</summary>
    public bool Contains(SceneEntity entity)
    {
        return _entities.ContainsKey(entity.Id);
    }

    /// <summary>清理所有注册。</summary>
    public void Clear()
    {
        _entities.Clear();
    }
}
