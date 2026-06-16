namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景事件总线——C# 侧事件分发。
/// 支持立即分发和延迟队列（deferred）模式。
/// 兼容旧的 SceneEventBus API，同时支持新的 ISceneEventBus 接口。
/// </summary>
public sealed class SceneEventBus : ISceneEventBus
{
    /// <summary>事件处理器委托。</summary>
    public delegate void SceneEventHandler(SceneEvent evt);

    /// <summary>按事件类型分组的订阅者。</summary>
    private readonly Dictionary<SceneEventType, List<SceneEventHandler>> _subscribers = new();

    /// <summary>延迟事件队列（在 FlushDeferred 时统一处理）。</summary>
    private readonly Queue<SceneEvent> _deferredQueue = new();

    /// <summary>全局订阅者（接收所有事件类型）。</summary>
    private readonly List<SceneEventHandler> _globalSubscribers = new();

    /// <summary>泛型事件订阅者（按类型分组）。</summary>
    private readonly Dictionary<Type, List<Delegate>> _genericSubscribers = new();

    /// <summary>泛型延迟事件队列。</summary>
    private readonly Queue<(Type EventType, object Event)> _genericDeferredQueue = new();

    /// <summary>是否正在分发事件（防止递归 Emit）。</summary>
    private bool _dispatching;

    // ── SceneEvent 订阅 ──

    /// <summary>订阅指定事件类型。</summary>
    public void Subscribe(SceneEventType type, SceneEventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (!_subscribers.TryGetValue(type, out var list))
        {
            list = new List<SceneEventHandler>();
            _subscribers[type] = list;
        }

        list.Add(handler);
    }

    /// <summary>订阅所有事件类型。</summary>
    public void SubscribeAll(SceneEventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);
        _globalSubscribers.Add(handler);
    }

    /// <summary>取消订阅指定事件类型。</summary>
    public void Unsubscribe(SceneEventType type, SceneEventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (_subscribers.TryGetValue(type, out var list))
        {
            list.Remove(handler);
        }
    }

    /// <summary>取消全局订阅。</summary>
    public void UnsubscribeAll(SceneEventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);
        _globalSubscribers.Remove(handler);
    }

    // ── SceneEvent 分发 ──

    /// <summary>立即分发事件（在当前调用栈上同步执行所有订阅者）。</summary>
    public void Emit(SceneEvent evt)
    {
        if (_dispatching)
        {
            // 防止递归：如果在事件处理中再次 Emit，入延迟队列
            _deferredQueue.Enqueue(evt);
            return;
        }

        _dispatching = true;
        try
        {
            Dispatch(evt);
        }
        finally
        {
            _dispatching = false;
        }
    }

    /// <summary>将事件入延迟队列，不在当前帧立即处理。</summary>
    public void EmitDeferred(SceneEvent evt)
    {
        _deferredQueue.Enqueue(evt);
    }

    // ── ISceneEventBus 泛型接口实现 ──

    /// <summary>即时分发泛型事件。</summary>
    public void Publish<T>(T evt) where T : struct
    {
        if (!_genericSubscribers.TryGetValue(typeof(T), out var list)) return;

        foreach (var handler in list)
        {
            try
            {
                ((Action<T>)handler)(evt);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[SceneEventBus] 事件处理异常 [{typeof(T).Name}]: {ex.Message}");
            }
        }
    }

    /// <summary>延迟分发泛型事件。</summary>
    public void PublishDeferred<T>(T evt) where T : struct
    {
        _genericDeferredQueue.Enqueue((typeof(T), (object)evt));
    }

    /// <summary>订阅泛型事件。</summary>
    public void Subscribe<T>(Action<T> handler) where T : struct
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (!_genericSubscribers.TryGetValue(typeof(T), out var list))
        {
            list = new List<Delegate>();
            _genericSubscribers[typeof(T)] = list;
        }

        if (!list.Contains(handler))
        {
            list.Add(handler);
        }
    }

    /// <summary>取消订阅泛型事件。</summary>
    public void Unsubscribe<T>(Action<T> handler) where T : struct
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (_genericSubscribers.TryGetValue(typeof(T), out var list))
        {
            list.Remove(handler);
        }
    }

    /// <summary>获取泛型事件订阅者数量。</summary>
    public int GetSubscriberCount<T>() where T : struct
    {
        return _genericSubscribers.TryGetValue(typeof(T), out var list) ? list.Count : 0;
    }

    // ── 刷新延迟队列 ──

    /// <summary>处理延迟队列中的所有事件。</summary>
    public void FlushDeferred()
    {
        // 处理 SceneEvent 延迟队列
        var count = _deferredQueue.Count;
        for (var i = 0; i < count; i++)
        {
            var evt = _deferredQueue.Dequeue();
            Emit(evt);
        }

        // 处理泛型事件延迟队列
        var genericCount = _genericDeferredQueue.Count;
        for (var i = 0; i < genericCount; i++)
        {
            var (eventType, eventObj) = _genericDeferredQueue.Dequeue();
            DispatchGenericEvent(eventType, eventObj);
        }
    }

    /// <summary>按标签处理延迟队列中的事件。</summary>
    public void FlushDeferred(SceneSystemTags mask)
    {
        // Phase 1: 忽略 mask，全量 Flush
        FlushDeferred();
    }

    /// <summary>延迟队列中待处理的事件数量。</summary>
    public int DeferredCount => _deferredQueue.Count + _genericDeferredQueue.Count;

    // ── 清理 ──

    /// <summary>清空所有订阅者和延迟队列。</summary>
    public void Clear()
    {
        _subscribers.Clear();
        _globalSubscribers.Clear();
        _deferredQueue.Clear();
        _genericSubscribers.Clear();
        _genericDeferredQueue.Clear();
    }

    /// <summary>仅清空延迟队列（保留订阅者）。</summary>
    public void ClearDeferred()
    {
        _deferredQueue.Clear();
        _genericDeferredQueue.Clear();
    }

    // ── 内部方法 ──

    private void Dispatch(SceneEvent evt)
    {
        // 按类型分发
        if (_subscribers.TryGetValue(evt.Type, out var list))
        {
            foreach (var handler in list)
            {
                handler(evt);
            }
        }

        // 全局订阅者
        foreach (var handler in _globalSubscribers)
        {
            handler(evt);
        }
    }

    private void DispatchGenericEvent(Type eventType, object eventObj)
    {
        if (!_genericSubscribers.TryGetValue(eventType, out var list)) return;

        foreach (var handler in list)
        {
            try
            {
                handler.DynamicInvoke(eventObj);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[SceneEventBus] 泛型事件处理异常 [{eventType.Name}]: {ex.Message}");
            }
        }
    }
}
