using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景世界——Gameplay Runtime Root。
/// 持有一个 Native 场景的完整 Managed 映射，管理该场景中的所有实体、查询和事件。
/// 不是 God Object：每个子系统（Entities / Queries / Systems / Events）各自负责，
/// SceneWorld 仅做组合和生命周期管理。
/// </summary>
public sealed class SceneWorld : IDisposable
{
    private bool _disposed;
    private NativeEventBridge? _nativeEventBridge;

    // ── 核心标识 ──

    /// <summary>Native 场景句柄（非零表示已创建）。</summary>
    public ulong NativeHandle { get; }

    /// <summary>场景名称。</summary>
    public string Name { get; set; }

    /// <summary>场景资产 GUID（可选，用于序列化和热重载）。</summary>
    public NNGuid AssetGuid { get; set; }

    // ── 子系统 ──

    /// <summary>实体注册表。</summary>
    public EntityRegistry Entities { get; }

    /// <summary>查询缓存。</summary>
    public SceneQueryCache Queries { get; }

    /// <summary>Managed System 调度器。</summary>
    public SceneSystemScheduler Systems { get; }

    /// <summary>场景事件总线。</summary>
    public SceneEventBus Events { get; }

    /// <summary>是否有效（Native 句柄非零且未释放）。</summary>
    public bool IsValid => NativeHandle != 0 && !_disposed;

    /// <summary>本场景跟踪的实体数量。</summary>
    public int EntityCount => Entities.Count;

    private SceneWorld(ulong nativeHandle, string name)
    {
        NativeHandle = nativeHandle;
        Name = name;
        Entities = new EntityRegistry(nativeHandle);
        Queries = new SceneQueryCache(nativeHandle);
        Systems = new SceneSystemScheduler(this);
        Events = new SceneEventBus();

        // 创建 Native 事件桥接（当前为 stub，待 ABI 扩展后自动桥接）
        _nativeEventBridge = new NativeEventBridge(nativeHandle, Events);
    }

    // ── 工厂方法 ──

    /// <summary>创建空场景世界。</summary>
    public static SceneWorld? Create(string name)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        var result = SceneNativeBridge.CreateScene(out var handle);
        if (result != NNSceneResult.Ok || handle == 0)
        {
            return null;
        }

        return new SceneWorld(handle, name);
    }

    // ── 实体操作（委托 EntityRegistry）──

    /// <summary>创建实体。</summary>
    public SceneEntity? CreateEntity(string? displayName = null) =>
        Entities.Create(displayName);

    /// <summary>销毁实体。</summary>
    public NNSceneResult DestroyEntity(SceneEntity entity) =>
        Entities.Destroy(entity);

    // ── Tick ──

    /// <summary>
    /// 主帧 Tick——按 TickGroup 顺序驱动所有 Managed System，中间穿插 Native System。
    /// <code>
    /// Managed EarlyUpdate → Managed FixedUpdate → Managed Update
    /// → Native TickSystems → Managed LateUpdate → Managed Render
    /// → Events.FlushDeferred
    /// </code>
    /// </summary>
    public void Tick(float deltaTime)
    {
        if (!IsValid)
        {
            return;
        }

        // Managed: EarlyUpdate
        Systems.Tick(TickGroup.EarlyUpdate, deltaTime);

        // Managed: FixedUpdate（固定步长由调用方决定）
        // 注意：FixedDeltaTime 由外部传入，此处使用 deltaTime
        // 若需独立 FixedTick，调用方应使用 FixedTick(float fixedDeltaTime)

        // Managed: Update
        Systems.Tick(TickGroup.Update, deltaTime);

        // Native System Tick（Native Scheduler 内部按其 TickGroup 调度）
        SceneNativeBridge.TickSystems(NativeHandle, deltaTime);

        // Managed: LateUpdate
        Systems.Tick(TickGroup.LateUpdate, deltaTime);

        // Managed: Render
        Systems.Tick(TickGroup.Render, deltaTime);

        // 处理延迟事件队列
        Events.FlushDeferred();
    }

    /// <summary>
    /// 固定步长 Tick——驱动 <see cref="ISystemFixedTick"/> 系统。
    /// 调用方负责以固定间隔调用此方法。
    /// </summary>
    public void FixedTick(float fixedDeltaTime)
    {
        if (!IsValid)
        {
            return;
        }

        Systems.FixedTick(TickGroup.FixedUpdate, fixedDeltaTime);
    }

    // ── 查询 ──

    /// <summary>获取或创建单组件查询。</summary>
    public SceneQuery<T> GetQuery<T>() where T : unmanaged =>
        Queries.GetQuery<T>();

    /// <summary>获取或创建双组件查询。</summary>
    public SceneQuery<T1, T2> GetQuery<T1, T2>()
        where T1 : unmanaged
        where T2 : unmanaged =>
        Queries.GetQuery<T1, T2>();

    // ── 序列化 ──

    /// <summary>序列化场景到 VFS 路径（VGSC 二进制格式）。</summary>
    public NNSceneResult Save(string vfsPath)
    {
        if (!IsValid)
        {
            return NNSceneResult.Invalid;
        }

        return SceneNativeBridge.SerializeScene(NativeHandle, vfsPath);
    }

    /// <summary>从 VFS 路径反序列化并创建世界。</summary>
    public static SceneWorld? LoadFromAsset(string name, string vfsPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(name);

        var result = SceneNativeBridge.DeserializeScene(out var handle, vfsPath);
        if (result != NNSceneResult.Ok || handle == 0)
        {
            return null;
        }

        var world = new SceneWorld(handle, name);
        return world;
    }

    // ── 热重载 ──

    /// <summary>
    /// 保存热重载状态快照。
    /// Native 场景数据不受 C# 程序集重载影响，此方法仅保存 Managed 侧映射。
    /// </summary>
    public HotReloadSnapshot SaveSnapshot()
    {
        return new HotReloadSnapshot
        {
            Name = Name,
            NativeHandle = NativeHandle,
            AssetGuid = AssetGuid,
            EntityHandles = Entities.ExportHandleValues(),
            CapturedAt = DateTimeOffset.UtcNow,
        };
    }

    /// <summary>
    /// 从热重载快照恢复场景世界。
    /// Native 场景句柄仍然有效，重建 Managed 侧映射即可。
    /// </summary>
    public static SceneWorld RestoreFromSnapshot(HotReloadSnapshot snapshot)
    {
        ArgumentNullException.ThrowIfNull(snapshot);

        var world = new SceneWorld(snapshot.NativeHandle, snapshot.Name);
        world.AssetGuid = snapshot.AssetGuid;

        // 从快照恢复实体注册表
        world.Entities.SyncFromHandles(snapshot.EntityHandles);

        return world;
    }

    /// <summary>
    /// 热重载后重建 Managed 侧状态。
    /// 关闭所有 System（调用 Shutdown），允许新程序集重新注册。
    /// Native 数据不受影响。
    /// </summary>
    public void RebuildAfterReload()
    {
        // 关闭旧 System（新程序集重新注册时会重新 Initialize）
        Systems.Rebuild();

        // 重建 Native 事件桥接
        _nativeEventBridge?.Dispose();
        _nativeEventBridge = new NativeEventBridge(NativeHandle, Events);
    }

    // ── 释放 ──

    /// <summary>释放场景世界及其所有 Native 资源。</summary>
    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        // 释放事件桥接
        _nativeEventBridge?.Dispose();
        _nativeEventBridge = null;

        // 关闭 System（逆序 Shutdown）
        Systems.Dispose();

        // 清空事件总线
        Events.Clear();

        // 清空查询缓存
        Queries.Clear();

        // 清空实体注册表
        Entities.Clear();

        // 销毁 Native 场景
        if (NativeHandle != 0)
        {
            SceneNativeBridge.DestroyScene(NativeHandle);
        }
    }
}
