using System.Collections.Concurrent;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// ISceneEventBus 的实现。
/// 支持即时分发和延迟队列两种模式。
/// </summary>
internal sealed class FrifloEventBus : ISceneEventBus
{
    private readonly Dictionary<Type, List<Delegate>> _handlers = new();
    private readonly ConcurrentQueue<(Type EventType, object Event)> _deferredQueue = new();
    private readonly List<(Type EventType, object Event)> _flushBuffer = new();
    private bool _isFlushing;

    // ── 即时分发 ──

    public void Publish<T>(T evt) where T : struct
    {
        if (!_handlers.TryGetValue(typeof(T), out var list)) return;

        foreach (var handler in list)
        {
            try
            {
                ((Action<T>)handler)(evt);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[EventBus] 事件处理异常 [{typeof(T).Name}]: {ex.Message}");
            }
        }
    }

    // ── 延迟分发 ──

    public void PublishDeferred<T>(T evt) where T : struct
    {
        _deferredQueue.Enqueue((typeof(T), (object)evt));
    }

    public void FlushDeferred()
    {
        if (_isFlushing) return;
        _isFlushing = true;

        try
        {
            _flushBuffer.Clear();
            while (_deferredQueue.TryDequeue(out var item))
            {
                _flushBuffer.Add(item);
            }

            foreach (var (eventType, eventObj) in _flushBuffer)
            {
                DispatchEvent(eventType, eventObj);
            }
        }
        finally
        {
            _isFlushing = false;
        }
    }

    // ── 订阅管理 ──

    public void Subscribe<T>(Action<T> handler) where T : struct
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (!_handlers.TryGetValue(typeof(T), out var list))
        {
            list = new List<Delegate>();
            _handlers[typeof(T)] = list;
        }

        if (!list.Contains(handler))
        {
            list.Add(handler);
        }
    }

    public void Unsubscribe<T>(Action<T> handler) where T : struct
    {
        ArgumentNullException.ThrowIfNull(handler);

        if (_handlers.TryGetValue(typeof(T), out var list))
        {
            list.Remove(handler);
        }
    }

    public void Clear()
    {
        _handlers.Clear();
        _flushBuffer.Clear();
        while (_deferredQueue.TryDequeue(out _)) { }
    }

    public int GetSubscriberCount<T>() where T : struct
    {
        return _handlers.TryGetValue(typeof(T), out var list) ? list.Count : 0;
    }

    // ── 内部方法 ──

    private void DispatchEvent(Type eventType, object eventObj)
    {
        if (!_handlers.TryGetValue(eventType, out var list)) return;

        foreach (var handler in list)
        {
            try
            {
                handler.DynamicInvoke(eventObj);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"[EventBus] 延迟事件处理异常 [{eventType.Name}]: {ex.Message}");
            }
        }
    }
}
