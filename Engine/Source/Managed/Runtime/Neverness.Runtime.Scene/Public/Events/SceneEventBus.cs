namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景事件总线——C# 侧事件分发。
/// 支持立即分发和延迟队列（deferred）模式。
/// 由 <see cref="SceneWorld.Events"/> 持有。
/// </summary>
public sealed class SceneEventBus
{
    /// <summary>事件处理器委托。</summary>
    public delegate void EventHandler(SceneEvent evt);

    /// <summary>按事件类型分组的订阅者。</summary>
    private readonly Dictionary<SceneEventType, List<EventHandler>> _subscribers = new();

    /// <summary>延迟事件队列（在 FlushDeferred 时统一处理）。</summary>
    private readonly Queue<SceneEvent> _deferredQueue = new();

    /// <summary>全局订阅者（接收所有事件类型）。</summary>
    private readonly List<EventHandler> _globalSubscribers = [];

    /// <summary>是否正在分发事件（防止递归 Emit）。</summary>
    private bool _dispatching;

    /// <summary>订阅指定事件类型。</summary>
    public void Subscribe(SceneEventType type, EventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (!_subscribers.TryGetValue(type, out var list))
        {
            list = new List<EventHandler>();
            _subscribers[type] = list;
        }

        list.Add(handler);
    }

    /// <summary>订阅所有事件类型。</summary>
    public void SubscribeAll(EventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);
        _globalSubscribers.Add(handler);
    }

    /// <summary>取消订阅指定事件类型。</summary>
    public void Unsubscribe(SceneEventType type, EventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (_subscribers.TryGetValue(type, out var list))
        {
            list.Remove(handler);
        }
    }

    /// <summary>取消全局订阅。</summary>
    public void UnsubscribeAll(EventHandler handler)
    {
        ArgumentNullException.ThrowIfNull(handler);
        _globalSubscribers.Remove(handler);
    }

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

    /// <summary>处理延迟队列中的所有事件。</summary>
    public void FlushDeferred()
    {
        var count = _deferredQueue.Count;
        for (var i = 0; i < count; i++)
        {
            var evt = _deferredQueue.Dequeue();
            Emit(evt);
        }
    }

    /// <summary>延迟队列中待处理的事件数量。</summary>
    public int DeferredCount => _deferredQueue.Count;

    /// <summary>清空所有订阅者和延迟队列。</summary>
    public void Clear()
    {
        _subscribers.Clear();
        _globalSubscribers.Clear();
        _deferredQueue.Clear();
    }

    /// <summary>仅清空延迟队列（保留订阅者）。</summary>
    public void ClearDeferred()
    {
        _deferredQueue.Clear();
    }

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
}
