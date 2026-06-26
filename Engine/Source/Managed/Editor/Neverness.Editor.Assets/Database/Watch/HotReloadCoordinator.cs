using Neverness.Editor.Core.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产变更事件类型。
/// </summary>
internal enum AssetChangeType
{
    Changed,
    Created,
    Deleted,
    Renamed
}

/// <summary>
/// 资产变更事件数据。
/// </summary>
internal readonly record struct AssetChangeEvent
{
    public AssetChangeType Type { get; init; }
    public NPath Path { get; init; }
    public NPath OldPath { get; init; } // Renamed 时使用

    public static AssetChangeEvent Changed(NPath path) => new() { Type = AssetChangeType.Changed, Path = path };
    public static AssetChangeEvent Created(NPath path) => new() { Type = AssetChangeType.Created, Path = path };
    public static AssetChangeEvent Deleted(NPath path) => new() { Type = AssetChangeType.Deleted, Path = path };
    public static AssetChangeEvent Renamed(NPath oldPath, NPath newPath) => new() { Type = AssetChangeType.Renamed, Path = newPath, OldPath = oldPath };
}

/// <summary>
/// 热重载协调器。
///
/// 负责将 AssetWatcher / ImportPipeline / Runtime NNAssetManager 三层接线：
///   1. AssetWatcher 检测文件系统变化
///   2. 事件入队，由 Tick() 从主线程出队处理
///   3. ImportPipeline 执行 reimport（含 DependencyGraph 脏传播）
///   4. 通过 C ABI 通知 Runtime 侧 MarkForReload + ReloadMarkedAssets
///
/// 使用方法：
///   var coordinator = new HotReloadCoordinator(assetsRoot);
///   coordinator.Start();
///   // 每帧调用
///   coordinator.Tick();
///   // ...
///   coordinator.Dispose();
///
/// @threadsafe AssetWatcher 事件从 Timer 回调触发，内部使用 lock(_lock) 保护事件队列。
///   Tick() 应由主线程调用，事件在主线程上处理。
/// </summary>
public sealed class HotReloadCoordinator : IDisposable
{
    private readonly AssetWatcher _watcher;
    private readonly HashSet<GUID> _pendingReloadGuids = new();
    private readonly Queue<AssetChangeEvent> _pendingEvents = new();
    private readonly object _lock = new();
    private bool _disposed;

    /// <summary>事件总线（可选，由 Install 后设置）。</summary>
    public IEditorEventBus? EventBus { get; set; }

    /// <summary>创建热重载协调器。</summary>
    /// <param name="assetsRoot">Assets 源目录。</param>
    /// <param name="debounceMilliseconds">防抖延迟毫秒数。</param>
    public HotReloadCoordinator(NPath assetsRoot, int debounceMilliseconds = 200)
    {
        /* 使用 ImportPipeline 的共享 ImportStateCache，避免双重缓存 */
        _watcher = new AssetWatcher(assetsRoot, debounceMilliseconds, ImportPipeline.StateCache);

        _watcher.OnAssetChanged += path => Enqueue(AssetChangeEvent.Changed(path));
        _watcher.OnAssetCreated += path => Enqueue(AssetChangeEvent.Created(path));
        _watcher.OnAssetDeleted += path => Enqueue(AssetChangeEvent.Deleted(path));
        _watcher.OnAssetRenamed += (oldPath, newPath) => Enqueue(AssetChangeEvent.Renamed(oldPath, newPath));
    }

    /* ======================== 公开 API ======================== */

    /// <summary>开始监听文件系统变化。</summary>
    public void Start()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);

        /* 确保 ImportPipeline 已初始化 */
        if (!ImportPipeline.IsInitialized)
        {
            Console.WriteLine("[HotReloadCoordinator] ImportPipeline 未初始化，跳过启动");
            return;
        }

        /* 注册导入器 */
        ImporterRegistry.Discover();

        _watcher.Start();
        Console.WriteLine("[HotReloadCoordinator] 热重载已启动");
    }

    /// <summary>停止监听。</summary>
    public void Stop()
    {
        _watcher.Stop();

        /* 停止时处理剩余事件并刷新 Runtime */
        Tick();
        Console.WriteLine("[HotReloadCoordinator] 热重载已停止");
    }

    /// <summary>是否正在监听。</summary>
    public bool IsActive => _watcher.IsWatching;

    /// <summary>
    /// 每帧由主线程调用——处理排队的变更事件并刷新 Runtime 重载请求。
    /// </summary>
    public void Tick()
    {
        if (_disposed) return;

        /* 出队所有待处理事件 */
        List<AssetChangeEvent> events;
        lock (_lock)
        {
            if (_pendingEvents.Count == 0 && _pendingReloadGuids.Count == 0)
                return;
            events = new List<AssetChangeEvent>(_pendingEvents);
            _pendingEvents.Clear();
        }

        /* 在主线程上处理事件 */
        foreach (var evt in events)
        {
            ProcessEvent(evt);
        }

        /* 批量刷新 Runtime 重载请求 */
        FlushRuntimeReloads();
    }

    /* ======================== 事件入队（AssetWatcher 线程调用） ======================== */

    /// <summary>将事件加入队列（线程安全，从 AssetWatcher Timer 回调调用）。</summary>
    private void Enqueue(AssetChangeEvent evt)
    {
        lock (_lock)
        {
            _pendingEvents.Enqueue(evt);
        }
    }

    /* ======================== 事件处理（主线程调用） ======================== */

    /// <summary>处理单个变更事件（在主线程上执行）。</summary>
    private void ProcessEvent(AssetChangeEvent evt)
    {
        try
        {
            switch (evt.Type)
            {
                case AssetChangeType.Changed:
                    HandleAssetChanged(evt.Path);
                    break;
                case AssetChangeType.Created:
                    HandleAssetCreated(evt.Path);
                    break;
                case AssetChangeType.Deleted:
                    HandleAssetDeleted(evt.Path);
                    break;
                case AssetChangeType.Renamed:
                    HandleAssetRenamed(evt.OldPath, evt.Path);
                    break;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] 处理事件失败: {evt.Type} {evt.Path} → {ex.Message}");
        }
    }

    /// <summary>文件内容变化 → Reimport + 脏传播 + 标记 Runtime reload。</summary>
    private void HandleAssetChanged(NPath sourcePath)
    {
        Console.WriteLine($"[HotReloadCoordinator] 资产变化: {sourcePath}");

        var result = ImportPipeline.Reimport(sourcePath);
        if (result != null && result.Success)
        {
            MarkForRuntimeReload(result.AssetGuid);
            PropagateDirtyAndReimport(result.AssetGuid, sourcePath);
            EmitReloaded(result.AssetGuid, sourcePath);
        }
    }

    /// <summary>新文件创建 → Import。</summary>
    private void HandleAssetCreated(NPath sourcePath)
    {
        Console.WriteLine($"[HotReloadCoordinator] 新资产: {sourcePath}");

        var result = ImportPipeline.Import(sourcePath);
        if (result != null && result.Success)
        {
            MarkForRuntimeReload(result.AssetGuid);
            EmitReloaded(result.AssetGuid, sourcePath);
        }
    }

    /// <summary>文件删除 → 清理 Database + Runtime unload。</summary>
    private void HandleAssetDeleted(NPath sourcePath)
    {
        Console.WriteLine($"[HotReloadCoordinator] 资产删除: {sourcePath}");

        /* 删除前先获取 GUID，用于 Runtime 卸载 */
        var virtualPath = new NVirtualPath(sourcePath.FullPath);
        EditorAssetDatabase.TryGetGuid(virtualPath, out var guid);

        /* 从 Database 移除 */
        EditorAssetDatabase.DeleteAsset(virtualPath);

        /* 从状态缓存移除 */
        ImportPipeline.StateCache.Remove(sourcePath);

        /* 通知 Runtime 卸载已加载的资产（幂等安全，未加载则跳过） */
        if (!guid.IsZero)
        {
            AssetHandle.Unload(guid);
            EmitDeleted(guid, sourcePath);
        }
    }

    /// <summary>文件重命名 → 移动 .meta + 清理旧缓存 + 更新 Database + reimport。</summary>
    private void HandleAssetRenamed(NPath oldPath, NPath newPath)
    {
        Console.WriteLine($"[HotReloadCoordinator] 资产重命名: {oldPath} → {newPath}");

        /* 移动 .meta 侧车文件 */
        MetaFileManager.MoveMeta(oldPath, newPath);

        /* 清理旧路径的 ImportStateCache 条目 */
        ImportPipeline.StateCache.Remove(oldPath);

        /* 更新 Database 映射 */
        EditorAssetDatabase.MoveAsset(new NVirtualPath(oldPath.FullPath), new NVirtualPath(newPath.FullPath));

        /* 新路径需要重新导入 */
        HandleAssetCreated(newPath);
    }

    /* ======================== 脏传播 ======================== */

    /// <summary>对变化资产执行 DependencyGraph 脏传播，reimport 所有反向依赖。</summary>
    private void PropagateDirtyAndReimport(GUID changedGuid, NPath sourcePath)
    {
        try
        {
            var dirtyAssets = ImportPipeline.DependencyGraph.GetDirtyPropagation(changedGuid);
            foreach (var dependentGuid in dirtyAssets)
            {
                if (dependentGuid == changedGuid)
                    continue;

                /* 通过 GUID 查找源路径并重新导入 */
                var depPath = ResolveSourcePath(dependentGuid);
                if (depPath != null)
                {
                    Console.WriteLine($"[HotReloadCoordinator] 脏传播重导入: {depPath}");
                    var depResult = ImportPipeline.Reimport(depPath.Value);
                    if (depResult != null && depResult.Success)
                    {
                        MarkForRuntimeReload(depResult.AssetGuid);
                    }
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[HotReloadCoordinator] 脏传播失败: {ex.Message}");
        }
    }

    /* ======================== Runtime 通知 ======================== */

    /// <summary>标记资产需要 Runtime 重载（批量收集）。</summary>
    private void MarkForRuntimeReload(GUID guid)
    {
        if (guid.IsZero) return;

        lock (_lock)
        {
            _pendingReloadGuids.Add(guid);
        }
    }

    /// <summary>批量刷新 Runtime 重载请求（Tick 时统一通知）。</summary>
    private void FlushRuntimeReloads()
    {
        List<GUID> guids;
        lock (_lock)
        {
            if (_pendingReloadGuids.Count == 0)
                return;
            guids = new List<GUID>(_pendingReloadGuids);
            _pendingReloadGuids.Clear();
        }

        Console.WriteLine($"[HotReloadCoordinator] 刷新 {guids.Count} 个 Runtime 重载请求");

        foreach (var guid in guids)
        {
            AssetHandle.MarkForReload(guid);
        }

        AssetHandle.ReloadMarkedAssets();
    }

    /* ======================== 辅助 ======================== */

    /// <summary>通过 EditorEventBus 发送资产重载事件。</summary>
    private void EmitReloaded(GUID guid, NPath path)
    {
        EventBus?.Emit(new EditorEvent(EditorEventType.AssetReloaded,
            new AssetReloadedEventPayload(guid, path)));
    }

    /// <summary>通过 EditorEventBus 发送资产删除事件。</summary>
    private void EmitDeleted(GUID guid, NPath path)
    {
        EventBus?.Emit(new EditorEvent(EditorEventType.AssetDeleted,
            new AssetReloadedEventPayload(guid, path)));
    }

    /// <summary>通过 GUID 解析源资产路径。</summary>
    private static NPath? ResolveSourcePath(GUID guid)
    {
        if (EditorAssetDatabase.TryGetPath(guid, out var virtualPath) && !virtualPath.IsEmpty)
            return new NPath(virtualPath.FullPath);
        return null;
    }

    /* ======================== IDisposable ======================== */

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        Stop();
        _watcher.Dispose();
    }
}

/// <summary>
/// 资产重载事件载荷。
/// 用于 AssetReloaded / AssetDeleted 事件的 Payload。
/// </summary>
public readonly record struct AssetReloadedEventPayload(GUID Guid, NPath Path);
